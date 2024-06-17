# umijs 3 升 4

> 有个老项目, 使用的前端框架为 umijs 3, 还有一个更老的项目使用的前端框架为 umijs 2, 前阵子把 umijs 3 升到了 4, 同时经过测试发布到了生产环境, 这里回顾并记录一下升级过程.

## 升级原因

umijs 4 已经发布有段时间了, [umijs 4](https://umijs.org/blog/umi-4-rc), 目前项目中经常遇到的问题就是编辑代码之后的等待, 以及有时候直接退出命令行, 这些是开发中的效率问题, 还有一部分是考虑升级时把一些项目目前不用的代码及多余库都给剔除, 保留目前系统要求的最小库, 这是其中一个目的, 还有一个目的是追加工程化的一些检查, 比如 ts eslint prettier 等强制要求, 杜绝项目中莫名其妙恶心的代码产生.

## 升级过程

> 整体思路为, 新建一个最新的 umijs 4 项目, 然后拷贝代码到最新项目, 直到运行成功, 基本功能完全没有问题, 再迁移到原项目中去

- `eslint` 配置文件包替换为 `@umijs/max/eslint`, 规则缩减
- 追加 `lintstage` 相关, 及调整
- `npmrc` 追加
- 删除 `nvmrc`
- `config.ts` 变更到 `umirc.ts` 中
- `config.ts` 内容调整, 参考 [升级到 Umi 4](https://umijs.org/docs/introduce/upgrade-to-umi-4)
- `webpack-chain` 插件相关配置全部去除, 其中相关包处理基本可以从项目中剔除
- `proxy` 相关配置不需要
- `mock` 数据及相关全部删除, 不需要
- `package.json` 替换为最新的 umijs 4 相关依赖, 同时追加项目中必不可少的差异包, 不包含再 `umijs/max` 中的
- 权限部分重写, 根据登录用户动态渲染路由, 根据动态渲染出的路由动态生成菜单, 追加 `access.ts` 重写细权限支持
- `app.ts` 按照原内容及之前页面配置进行重写
- 部分组件改写, `类组件` 改写为 `函数组件`, 使用 `hook` 获取相关资源, 而不是 `props`
- 子组件改写, `Outlet`
- 代码中包相关替换 `@umijs/max`
- `antd`, `antd pro`, `antd icon` 升到最新, 去除 `less` 相关依赖
- `国际化` 不用, 暂时全部剔除
- `dwa` 处理 `namepace` 冲突, 排除法
- 替换 `PageHeaderWrapper` 为 `PageContainer`
- 弃用 `umi-request`, 使用 `axios`
- `import * as moment from 'moment'` 替换为 `import moment from 'moment'`
- `tsconfig.json` 调整, 去掉多余配置

## 后记

以上内容可能对于大部分同样升级 `umijs3` 到 `umijs4` 的用户参考价值并不大, 一般来说给出对应报错, 及处理方法比较稳妥, 但是升级时遇到的报错及问题, 基本都找相关信息给处理掉了, 下次最好边升级边记录, 这样对后面要参考的同好帮助大一些. 以此为戒, 感谢 `Telegram` `UmiJS 官方交流群` 各位老师的帮助和解答.
