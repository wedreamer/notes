# eShopOnContainers

1. 整体架构
- client apps
    - mobile app
    - traditional web app
    - spa web app

- docker host
    - web app mvc
    - identity service
    - api gateway
    - bff
        - microservice
        - catalog
        - order
        - basket

2. 目录结构

- 仓库目录

```yaml
./eShopOnContainers
├── CONTRIBUTING.md
├── LICENSE
├── README.md
├── branch-guide.md # 分支介绍
├── build # build 脚本
├── deploy # 发布需要的资源
├── github-actions.md # github action 介绍
├── img # 图片
└── src # 源代码
```

- 源码目录

```yaml
./eShopOnContainers/src
├── ApiGateways # 网关
├── BuildingBlocks 
├── Create-DockerfileSolutionRestore.ps1 # 脚本 -> 创建 dockerfile 解决方案
├── DockerfileSolutionRestore.txt
├── Mobile # 移动端介绍 -> 对应的项目地址
├── NuGet.config # nuget 配置文件
├── Services # 服务
├── Tests # 测试
├── Web # web 端, 包含 mvc spa webHookClient webStatus 
├── docker-compose-tests.override.yml # 相关 build 需要的资源
├── docker-compose-tests.yml
├── docker-compose-windows.prod.yml
├── docker-compose.dcproj
├── docker-compose.elk.yml
├── docker-compose.override.windows.yml
├── docker-compose.override.yml
├── docker-compose.prod.yml
├── docker-compose.windows.yml
├── docker-compose.yml
├── eShopOnContainers-ServicesAndWebApps.sln
├── package-lock.json
├── prepare-devspaces.ps1
└── run-test-infrastructure.ps1
```

3. 代码模块

服务

```yaml
./eShopOnContainers/src/Services
├── Basket # 购物车
├── Catalog # 目录
├── Identity # 身份
├── Ordering # 订单
├── Payment # 支付
└── Webhooks # webHook
```

4. 服务

- 购物车
- 分类
- 身份
- 订单
- 支付
- webHook

5. 购物车

目录

```yaml
./eShopOnContainers/src/Services/Basket
├── Basket.API # Api
├── Basket.FunctionalTests # 函数式测试
└── Basket.UnitTests # 单元测试
```

Api

```yaml
./eShopOnContainers/src/Services/Basket/Basket.API
├── Auth # 认证
├── Basket.API.csproj
├── BasketSettings.cs
├── Controllers
├── Dockerfile
├── Dockerfile.develop # dev 环境
├── GlobalUsings.cs # 全局引用的命名空间
├── Grpc # grpc 服务
├── Infrastructure # 基础设施
├── IntegrationEvents # 集成事件
├── Model
├── Program.cs
├── Properties
├── Proto
├── README.md
├── Services
├── Startup.cs
├── TestHttpResponseTrailersFeature.cs
├── appsettings.Development.json
├── appsettings.json
├── azds.yaml
├── bin
├── obj
├── values.dev.yaml
└── web.config
```
