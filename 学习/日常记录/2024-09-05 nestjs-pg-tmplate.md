# nestjs-pg-tmplate

> 经常初始化项目, 有些功能都是通用的, 每次都进行通用化功能的实现比较繁琐, 因此提取到该项目中.
> [github nestjs-pg-tmplate](https://github.com/wedreamer/nestjs-pg-tmplate)

## 相关功能

- ts strict true
- debugger vscode
- eslint, prettier, cSpell, commitLint, husky
- config yml env 支持
- validate and transform
- docker compose pg redis
- typeorm migration soft-delete
- user login register rbac
- app metaData
- open sdk generate
- logger dayjs

## ts strict true

```jsonc
{
  // ...
  "strict": true,
  "strictNullChecks": true,
  "noImplicitAny": false,
  "strictBindCallApply": true,
  "forceConsistentCasingInFileNames": true,
  "noFallthroughCasesInSwitch": true
  // ...
}
```

## debugger vscode

```jsonc
// .vscode/settings.json
{
  "editor.detectIndentation": false,
  "editor.tabSize": 2,
  "editor.indentSize": 2,
  "editor.defaultFormatter": "esbenp.prettier-vscode",
  "[typescript]": {
    "editor.defaultFormatter": "esbenp.prettier-vscode"
  }
}
```

```jsonc
// .vscode/launch.json
{
  "version": "0.2.0",
  "configurations": [
    {
      "type": "node",
      "request": "attach",
      "name": "Attach NestJS WS",
      "port": 9229,
      "restart": true
    }
  ]
}
```

```json
// .vscode/extensions.json
{
  "recommendations": [
    "dbaeumer.vscode-eslint", // ESLint 插件
    "esbenp.prettier-vscode", // Prettier 插件
    // "rangav.vscode-thunder-client", // thunder client
    "streetsidesoftware.code-spell-checker" // spell
  ]
}
```

## eslint, prettier, cSpell, commitLint, husky

prettier 相关更改

```js
endOfLine: 'auto' // 主要针对不同系统换行及结尾问题
```

eslint, prettier 默认 nestjs 项目初始化会自动安装, 仅需针对配置文件进行相关更新即可

[commitLint husky 对应提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/31397fec0ee00103602390edf07a19bc29bc80b2)

```jsonc
// npx husky init
"prepare": "husky"
```

```bash
pnpm i @commitlint/cli @commitlint/config-conventional husky -D
```

```js
// commitlint.config.js
module.exports = {
  extends: ['@commitlint/config-conventional'],
}
```

```bash
# .husky/pre-commit
pnpm test
pnpm lint

# .husky/commit-msg
npx --no -- commitlint --edit "$1"
```

## config yml env

> 这里未将项目配置单独抽成相关包, 同时针对配置类型进行了严格校验

[config yml env 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/1825f4b0b7e0bdb741104a939e08ba2a433d363e)

```bash
pnpm add --save class-validator class-transformer @nestjs/config js-yaml
pnpm add @types/js-yaml -D
```

```ts
// src/config/config.ts
import {
  IsEnum,
  IsNumber,
  IsOptional,
  IsString,
  Validate,
  ValidateIf,
} from 'class-validator'

export enum Env {
  dev = 'dev',
  pro = 'pro',
}

export class db {
  @IsString()
  host!: string

  @IsNumber()
  port!: number

  @IsString()
  username!: string

  @IsString()
  password!: string

  @IsString()
  database!: string
}

export class Security {
  @IsString()
  jwtKey!: string

  /** expressed in seconds or a string describing a time span [zeit/ms](https://github.com/zeit/ms.js).  Eg: 60, "2 days", "10h", "7d" */
  @IsOptional()
  @ValidateIf((_, val) => typeof val === 'number')
  @IsNumber()
  @ValidateIf((_, val) => typeof val === 'string')
  @IsString()
  jwtExpiresIn?: number = 30 * 60
}

export class Config {
  @IsOptional()
  @IsEnum(Env)
  env: Env = Env.dev

  @IsOptional()
  @IsString()
  appName: string = 'server'

  @Validate(() => Security)
  security!: Security

  @Validate(() => db)
  db!: db
}
```

```ts
// src/config/configuration.ts
import { readFileSync } from 'fs'
import * as yaml from 'js-yaml'
import { join } from 'path'
import { Config } from './config'
import { plainToInstance } from 'class-transformer'
import { validateSync } from 'class-validator'

const YAML_CONFIG_FILENAME = 'config.yml'

let config: Config | undefined

export default () => {
  if (config) return config
  const configData = yaml.load(
    readFileSync(join(process.cwd(), YAML_CONFIG_FILENAME), 'utf8')
  ) as Record<string, any>
  // transform and valiate
  const target = plainToInstance(Config, configData)
  const error = validateSync(target)
  if (error.length > 0) {
    throw new Error(error.map(err => err.toString(true)).join('\n'))
  } else {
    config = target
    return target
  }
}
```

## validate and transform

> 极其重要, 主要是验证和转换规则

```ts
// src/main.ts
import { NestFactory } from '@nestjs/core'
import { AppModule } from './app.module'
import { ValidationPipe } from '@nestjs/common'

async function bootstrap() {
  const app = await NestFactory.create(AppModule)

  app.useGlobalPipes(
    new ValidationPipe({
      transform: true,
      transformOptions: {
        strategy: 'excludeAll',
        enableImplicitConversion: true,
        excludeExtraneousValues: true,
      },
    })
  )
  app.enableCors()

  await app.listen(3000)
}
bootstrap()
```

## docker compose pg redis

> 环境依赖安装, 主要针对本地开发环境, 生产环境则自行考虑, 这里增加 redis 是因为 redis 常用于发布订阅或缓存, 但这些功能模板并未实现, 后续考虑

[docker compose pg redis 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/b076433ebe72eb3ecc0693e6b5496776c661c05c)

```bash
# .gitignore
volumes
```

```yml
# docker-compose-depend.yml
services:
  redis:
    image: redis:latest
    container_name: redis
    restart: unless-stopped
    privileged: true
    volumes:
      - ./volumes/redis/datadir:/data
      - ./volumes/redis/conf/redis.conf:/usr/local/etc/redis/redis.conf
      - ./volumes/redis/logs:/logs
    command:
      # redis-server
      #  两个写入操作 只是为了解决启动后警告 可以去掉
      /bin/bash -c "echo 511 > /proc/sys/net/core/somaxconn && echo never > /sys/kernel/mm/transparent_hugepage/enabled && redis-server /usr/local/etc/redis/redis.conf"
    ports:
      - 6379:6379
    networks:
      - dev

  pgsql:
    image: postgres:latest
    restart: unless-stopped
    container_name: pgsql-new
    environment:
      - POSTGRES_USER=postgres
      - POSTGRES_PASSWORD=postgres
      - TZ=Asia/Shanghai
    ports:
      - '5434:5432'
    volumes:
      # 初始的数据文件
      - ./volumes/pgsql/data:/var/lib/postgresql/data
    networks:
      - dev
networks:
  dev:
    name: local-dev
```

```bash
# redis pgsql
docker compose -f docker-compose-depend.yml up -d
```

## typeorm pg

> 这里主要抽 db 模块出来作为 lib, 支持 typeorm 及 基础实体, 软删除, 迁移, 回滚, 实体同步等相关功能

[typeorm pg 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/1548501b39f9a1a485a2d57e6ad45f4962e95238)

```bash
nest g lib db
```

```bash
pnpm add --save @nestjs/typeorm typeorm pg
```

## user login register rbac

> 需要进一步考虑, 这里的权限其实就是接口的元数据信息, 角色组合权限, 用户的权限则是角色权限和用户权限的合集

[user login register rbac 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/5577f700243463288f51cc429f0db869d8cf632e)

```bash
pnpm add bcrypt @nestjs/jwt @nestjs/passport passport-jwt
```

```bash
nest g mo auth
nest g co auth
nest g s auth

nest g mo user
nest g co user
nest g s user

nest g mo role
nest g co role
nest g s role
```

代码较多, 仅展示应用元数据相关实现, 其他移步 [代码库相关提交](https://github.com/wedreamer/nestjs-pg-tmplate/tree/5577f700243463288f51cc429f0db869d8cf632e) 查看

```ts
import { Controller, Get } from '@nestjs/common'
import { DiscoveryService, Reflector } from '@nestjs/core'
import { Describe, DESCRIBE_KEY } from 'src/_decorator/describe'
import { ControllerInfo } from './dto/meta.dto'

// 做进程内缓存, 避免重复计算
let allControllerInfo: ControllerInfo[] | undefined

/**
 * 权限管理
 */
@Controller('meta')
@Describe('权限管理')
export class MetaController {
  constructor(
    private readonly discoveryService: DiscoveryService,
    private readonly reflector: Reflector
  ) {}

  /**
   * 获取所有权限信息
   * @returns 权限信息
   */
  @Get()
  @Describe('所有权限信息')
  allMetaData(): ControllerInfo[] {
    if (allControllerInfo) return allControllerInfo
    const controllers = this.discoveryService.getControllers()
    const result: ControllerInfo[] = []

    controllers.forEach(controller => {
      const name = controller.name
      const describe = this.reflector.get(DESCRIBE_KEY, controller.metatype)
      if (!describe) return
      const methods = Object.getOwnPropertyNames(controller.metatype.prototype)
      const resMethods = methods
        .map(method => {
          if (method !== 'constructor') {
            const methodDescribe = Reflect.getMetadata(
              DESCRIBE_KEY,
              controller.metatype.prototype[method]
            )
            if (methodDescribe)
              return {
                name: method,
                describe: methodDescribe,
                context: `${name}/${method}`,
              }
          }
        })
        .filter(Boolean) as ControllerInfo['methods']
      result.push({ name, describe, methods: resMethods, context: name })
    })

    allControllerInfo = result

    return result
  }
}
```

## open sdk generate

> 这里使用 openapi 分支进行文档书写及生成 sdk, 目前不发 npm 包而是使用子仓库的方式引用

[open sdk generate 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/49497883a95662b6472c47fb9ae152666d2f5643)

```bash
pnpm add @nestjs/swagger @hey-api/openapi-ts @hey-api/client-fetch -D
```

```bash
# change branch to openapi
npm version patch
git push --tags
git push
```

## dayjs and logger

> 日志相关

[dayjs and logger 相关提交记录](https://github.com/wedreamer/nestjs-pg-tmplate/tree/4b951d4f12831c9959aed8251439da2ffd4be972)

```bash
pnpm i winston nest-winston dayjs
```

```ts
import * as winston from 'winston'
import {
  utilities as nestWinstonModuleUtilities,
  WinstonModule,
} from 'nest-winston'
import * as path from 'path'
import configuration from 'src/config/configuration'
import { Env } from 'src/config/config'

const transports = {
  console: new winston.transports.Console({
    level: 'silly',
  }),
  logFile: new winston.transports.File({
    dirname: path.join(process.cwd(), 'logs'),
    filename: 'log.json',
    level: 'silly',
  }),
}

const getLogger = async () => {
  const config = configuration()
  // 如果是本地测试环境则启用 nest 风格的 log format
  if (config.env == Env.dev) {
    transports.console = new winston.transports.Console({
      level: 'silly',
      format: winston.format.combine(
        winston.format.timestamp({
          format: () =>
            new Date().toLocaleString('chinese', {
              timeZone: 'Asia/Shanghai',
              hour12: false,
            }),
        }),
        nestWinstonModuleUtilities.format.nestLike(config.appName)
      ),
    })
  }
  const logger = WinstonModule.createLogger({
    transports: [transports.console, transports.logFile],
    level: 'silly',
  })
  return logger
}

export default getLogger
```

## 数据迁移

```bash
# 生成迁移 (根据实体信息)
npm run typeorm migration:generate -n ./libs/db/src/migrations/fix-data
# 新建迁移
npx typeorm migration:create ./libs/db/src/migrations/init-data
# 迁移
npm run typeorm migration:run --fake
# 回滚
npm run typeorm migration:revert
# 同步实体
npm run typeorm schema:sync
```

## 提交代码

```bash
pnpm lint
# git commit -m "type: subject"
# rules view https://www.npmjs.com/package/@commitlint/config-conventional
```
