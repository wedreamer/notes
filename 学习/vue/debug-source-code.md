# debug vue3 源码 

## 思路

只要让 vue3 生成 sourcemap 代码, 而使用的项目模板, 比如 vue 官方提供的模板, 或者是原始 `index.html` 类似项目引用相关包或者库即可, 前者大概率是包共享, vue3 源码 build 之后 link 到全局, 在你的项目中去除官方 `vue 远程包` link 本地包即可

## 相关调试配置及调整

目前源码构建使用的是 `rollup`, 需要 `sourcemap`, 即在 `output` 相关配置上增加, 位置在 `vue/scripts/config.js`

```ts
output: {
    file: opts.dest,
    format: opts.format,
    banner: opts.banner,
    name: opts.moduleName || 'Vue',
    exports: 'auto',
    + sourcemap: true
}
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
            "url": "file:///C:/Users/14094/code/vue-debug/raw/index.html",
            "webRoot": "${workspaceFolder}/raw",
            "outFiles": [
                "${workspaceFolder}/vue/**/*.js",
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
mkdir vue-debug
cd vue-debug
git clone git@github.com:vuejs/vue.git
cd vue
pnpm i

# raw project
mkdir raw
touch index.html

# vue-project
pnpm create vue@latest
```


TODO: 调试视频
