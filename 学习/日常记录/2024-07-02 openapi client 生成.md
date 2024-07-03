# openapi client 生成

> 最近尝试了一些根据 openapi 元数据生成 sdk 的库, 比如 `autorest`, 官方的 `swagger-codegen-cli`, 自己最终选用的 `@hey-api/openapi-ts`, 以下记录一下过程, 及选择的理由.

## sdk 自动生成

为什么需要自动生成, 一般来说后端接口会写对应的 `swagger` 文档, 构造相关元数据, 然后喂给 `swagger ui` 即可生成前端可使用可测试的网页, 实际上根据元数据就可以生成`相关语言的 sdk`, 这得益于 `api 的标准化`, 而这个标准化过程中的主力是 `openapi`. 同时生成 sdk 的过程是否可以增加相关`基本校验`？ 如果可以的话, 前端的基本校验功能就可以在 sdk 中无成本引入, 常见的 `400` 尤其是数据格式的报错将会大大减少. 当然这还是理想中的情况, 实际上现在生成 sdk 或者代码基本都没有`参数校验功能`的自动生成, 个人感觉是这块标准化的不足, 以及`校验库`可选的太多, 大家又并不一定有同样的标准. 大概这些原因, 自动生成 `sdk` 或者代码的话, 对后端构建接口而言, 只需要设计好`输入`及`返回结果`的相关类型, 如果考虑的更周到一些, 对应`文档注释`, 以及`示例`完善一下, 大部分主流框架这个时候基本都能生成对应的 `openapi` 元数据, 根据元数据完成相关 `sdk` 或者代码的生成, 不仅前端可以用, `后端跨服务调用`也是 ok 的, 但是服务之间的调用大多数并不是 `http`, 很可能是 `grpc` 之类的. 而且对于接口的开发过程中, 只需要完成接口功能细节, `输入输出`可能基本不会变, 即使变了生成的成本可能也很低, 尤其是接入相关构建流水线`只会更方便`. 对前端来说的话, 不用一个一个写服务请求, 更可能来公共请求类可能都不用写, 这是其中一方面, 更重要的是`类型复用`, 生成的 `sdk` 或者代码, 相关 `dto` 往往正好对应着前端的`产物类型`, 使用得当确实可以`节省`前端很多时间, 提高很多`效率`. 额外说一嘴, 这种前后端的工作交互方式对后端构造接口要求稍微高一些, 尤其是对于有些后端程序员, `什么样的输入可能导致什么样的输出都不明确`, `类型不校验`, `该提示不提示`, `该返回的必要信息不返回`. 以后有时间再详谈.

## @hey-api/openapi-ts

为啥不用 `autorest`, 这个包蛮好的, 就是很难调, 可以直接根据元数据生成代码, 而且基本上生成的状态是发包前一步了, 如果发 `npm` 包, 流水线上基本只需要 `npm publish` 带上 token 就发好了. 这个包生成的代码很难调用, 当然其本身设计上偏向于 `azure` 云环境.

`swagger-codegen-cli` 生成的代码比较简洁, 不过调用会出现问题, 尤其是前端调的时候, 有些依赖项并没有, 会出现 `undefined`, 依赖了个 node 的 `url` 包, 作者可能当时并没有考虑到浏览器环境, 也可能是时间很久没有更新了.

`@hey-api/openapi-ts` 这个包前身是 `openapi-typescript-codegen`, 后者笔者在一个 `.net web api` 项目中使用过, 生成对应 sdk 提供给前端使用, 预期效果很好. `@hey-api/openapi-ts` 的使用体验基本差不多, 以下直接提供相关配置过程及使用过程.

## 使用过程

环境为 `nestjs`, 前端为 `vue`, ci 环境为 `github action`

- `openapi` 元数据获取

> 这里实际在流水线中启动一个服务, 然后去 `请求相关元数据`, 目前没有好的 `build` 方法, 理论上可以直接 `build` 出来, 不需要启动服务, `build` 过程中有一个比较重要的插件配置

- `nest-cli.json`
```json
"compilerOptions": {
    "plugins": [
      {
        "name": "@nestjs/swagger",
        "options": {
          // 这里使用是主要的类型 dto(视图) entity(领域实体) schema(存储实体)
          "dtoFileNameSuffix": [".dto.ts", ".entity.ts", ".schema.ts"],
          "classValidatorShim": true,
          "introspectComments": true
        }
      }
    ],
```
- `package.json`

```bash
pnpm add @hey-api/openapi-ts -D
```

```json
"gen:openapi-ts": "npx @hey-api/openapi-ts -i /tmp/openapi/swagger-spec.json -o /tmp/openapi/sdk -c @hey-api/client-fetch",
```

- 相关 `流水线` (增加相关注释, 方便读者快速浏览)

```yml
name: Generate and Push SDK

# 只有打 tag 才触发
on:
  push:
    # branches:
    #   - openapi
    tags:
      - 'v*'

jobs:
  build-and-publish:
    runs-on: ubuntu-latest

    # 该仓库代码 Checkout
    steps:
    - name: Checkout code
      uses: actions/checkout@v2

    # node 20 环境
    - name: Use Node.js
      uses: actions/setup-node@v2
      with:
        node-version: '20'
        registry-url: 'https://registry.npmjs.org'

    # 依赖安装及启动服务(为了获取服务的 openapi 元数据)
    - name: Install dependencies and start server
      run: |
        npm i
        cp config.example.yml config.yml
        npm run start &
        sleep 10
    
    # 获取元数据并保存到指定位置
    - name: Download swagger.json
      run: |
        mkdir -p /tmp/openapi
        curl http://localhost:3000/api-json > /tmp/openapi/swagger-spec.json
    
    # 调用脚本生成 sdk 代码
    - name: Generate SDK
      run: |
        npm run gen:openapi-ts

    # 这里不使用 npm 包的方式发布, 而是使用子仓库, 带着 token Checkout 目标仓库
    - name: Checkout target repository
      uses: actions/checkout@v2
      with:
        repository: 'wedreamer/lims-server-openapi'
        token: ${{ secrets.LIMS_SERVER_OPENAPI_GITHUB_TOKEN }}

    # 删除上次代码, 这个仓库的所有代码, 除了本地的 .git, 完全使用生成的 sdk 代码
    - name: Copy SDK to target repository
      run: |
        find . -not -name .git -not -path './.git/*' -delete
        cp -r /tmp/openapi/sdk/* ./

    # push 到子仓库
    - name: Push SDK to target repository
      run: |
        git config --global user.email "shubuzuo@gmail.com"
        git config --global user.name "shubuzuo"
        cd .
        git add .
        git commit -m "Update SDK"
        git push
      env:
        GIT_TOKEN: ${{ secrets.LIMS_SERVER_OPENAPI_GITHUB_TOKEN }}
```

- 前端 `vue`

```bash
# 因为生成包时选择了基于该包的方式, 所以需要该包进行配置, 建议使用这个包, 生成时 -c @hey-api/client-fetch
pnpm add @hey-api/client-fetch
```

`client.ts` 封装一下

```ts
import { createClient } from '@hey-api/client-fetch'

const getToken = () => `Bearer ${localStorage.getItem('token') ?? ''}`

const openapi = createClient({
  baseUrl: 'http://localhost:3000',
  headers: {
    Authorization: getToken()
  }
})

openapi.interceptors.request.use((request, options) => {
  // 登录之后 token 拦截器中附带
  request.headers.set('Authorization', getToken())
  return request
})

openapi.interceptors.response.eject((res, req, options) => {
  // 集中错误处理
  // TODO: options 中允许设置不进行错误处理, 直接给到程序逻辑进行处理
  return res
})

export default openapi
```

- `store` 中使用

```ts
import { ref } from 'vue'
import { defineStore } from 'pinia'
import { authGetProfile, authLogin, type loginDto, type loginResDto, type User } from '@/openapi'
import openapi from '@/client'

export const useUserStore = defineStore('user', () => {
  const current = ref<User | null>(null)

  const getCurrent = async () => {
    const { data: me, error } = await authGetProfile({ client: openapi })
    if (!error) {
      current.value = me!
    }
  }

  const login = async (dto: loginDto): Promise<loginResDto | null> => {
    const { data, error } = await authLogin({ client: openapi, body: dto })
    if (!error) {
      const { access_token: accessToken = '' } = data as loginResDto
      localStorage.setItem('token', accessToken)
      await getCurrent()
      return data!
    }
    return null
  }

  const logout = () => {
    current.value = null
    localStorage.setItem('token', '')
    // TODO: req to cancel token
  }

  return { current, getCurrent, login, logout }
})
```

基本使用体验还是蛮 **优雅** 的


