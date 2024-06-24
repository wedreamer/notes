# debug vue3 源码

## 思路

只要让 vue3 生成 sourcemap 代码, 而使用的项目模板, 比如 vue 官方提供的模板, 或者是原始 `index.html` 类似项目引用相关包或者库即可, 前者大概率是包共享, vue3 源码 build 之后 link 到全局, 在你的项目中去除官方 `vue 远程包` link 本地包即可

## 直接运行

```bash
# 不用更改任何配置
pnpm i 
pnpm dev
# 可以看到 packages/vue/dist/ 出现的产物, 及 map 文件
```

```json
{
    "version": "0.2.0",
    "configurations": [
        // 调试 npm run build 或者类似脚本
        {
            "name": "vue build",
            "request": "launch",
            "runtimeArgs": [
                "run",
                "build"
            ],
            "runtimeExecutable": "npm",
            "skipFiles": [
                "<node_internals>/**"
            ],
            "type": "node",
            "cwd": "${workspaceFolder}/vue"
        },
        // 远程 url 调试, 这里面要注意 outFiles, 此处未加, 实际需要加
        {
            "type": "msedge",
            "request": "launch",
            "name": "vue-project",
            "url": "http://localhost:5173/",
            "webRoot": "${workspaceFolder}/vue-project"
        },
        // 原始 index.html 调试, 同上个原理一致
        {
          "type": "msedge",
          "request": "launch",
          "name": "raw-html",
          "url": "${workspaceFolder}/temp/index.html",
          "webRoot": "${workspaceFolder}/temp",
          "outFiles": [
            "${workspaceFolder}/packages/**/*.js",
          ]
        }
    ]
}
```

## 完整过程

- vue 项目
- 创建原生 html
- 创建 vue 模板项目

```bash
git clone git@github.com:vuejs/core.git
cd core
pnpm i
pnpm dev

mkdir -p temp && touch temp/index.html

# vue-project
pnpm create vue@latest
```
http://mongodbtoolchain.build.10gen.cc/toolchain/ubuntu2204/x86_64/latest
http://mongodbtoolchain.build.10gen.cc/toolchain/ubuntu2204/x86_64/mongodbtoolchain-c8946f1ee23987ed32481a2f571d0ee38f86667b.tar.gz

TODO: 调试视频
