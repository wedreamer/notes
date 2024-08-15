# node 后端 dockerFile 示例

> 不废话直接粘贴吧

## 不怎么依赖其他环境, 简单的 `dockerfile`, `分层构建`, 先构建 `依赖`, 再构建 `产物`, 再组合 `两者`, 力求 `最小` 的 `镜像大小`

```dockerfile
# dev 产物依赖包
FROM node:lts-bullseye-slim As development

USER root

WORKDIR /usr/src/app

COPY --chown=root:root package*.json pnpm-lock.yaml .npmrc ./

RUN npm i pnpm -g --registry=https://registry.npmmirror.com/
RUN pnpm i

COPY --chown=root:root . .

USER node

# build 产物目标文件
FROM node:lts-bullseye-slim As build

WORKDIR /usr/src/app

COPY --chown=root:root package*.json pnpm-lock.yaml .npmrc ./

COPY --chown=root:root --from=development /usr/src/app/node_modules ./node_modules

COPY --chown=root:root . .

RUN npm run build

ENV NODE_ENV production

# 运行环境本身, 及整合 dev 依赖及 build 目标程序文件
FROM node:lts-bullseye-slim As production

USER root

WORKDIR /usr/src/app

COPY --chown=root:root --from=development /usr/src/app/node_modules ./node_modules

# 运行时的其他依赖
COPY --chown=root:root --from=build /usr/src/app/bin ./bin
COPY --chown=root:root --from=build /usr/src/app/templates ./templates
COPY --chown=root:root --from=build /usr/src/app/public ./public
COPY --chown=root:root --from=build /usr/src/app/dist ./dist

HEALTHCHECK --interval=2s --timeout=5s CMD curl -f http://localhost:5000/api/app/ping || exit 1
EXPOSE 5000

CMD [ "node", "dist/main.js" ]
```

## 需要依赖其他环境, 稍微复杂的 `dockerfile`

> 这里依赖环境为 `canvas` 的 `linux` build 依赖, `python` 运行依赖, 及相关依赖包, 其实可以考虑使用 `pyenv` 要好一些, 锁包版本, 还有对应的环境字体, 实际上`时区时间` 也可以加上, 只不过此处没有展示.

```dockerfile
###################
# BUILD FOR LOCAL DEVELOPMENT
###################

FROM node:lts-bullseye-slim As development

USER root
RUN cp /etc/apt/sources.list /etc/apt/sources.list.bak; \
  echo " " > /etc/apt/sources.list; \
  echo "deb http://mirrors.cloud.tencent.com/debian/ bullseye main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.cloud.tencent.com/debian/ bullseye-updates main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.cloud.tencent.com/debian/ bullseye-backports main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.cloud.tencent.com/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list; 

RUN apt-get update;
RUN apt-get install --assume-yes apt-utils;
# canvas -> https://github.com/Automattic/node-canvas/wiki/Installation%3A-Ubuntu-and-other-Debian-based-systems
RUN apt-get install build-essential libcairo2-dev libpango1.0-dev libjpeg-dev libgif-dev librsvg2-dev -y

# Create app directory
WORKDIR /usr/src/app

# Copy application dependency manifests to the container image.
# A wildcard is used to ensure copying both package.json AND package-lock.json (when available).
# Copying this first prevents re-running npm install on every code change.
COPY --chown=root:root package*.json pnpm-lock.yaml .npmrc ./

# # Install app dependencies using the `npm ci` command instead of `npm install`
RUN npm i pnpm -g --registry=https://registry.npmmirror.com/
RUN pnpm i

# Bundle app source
COPY --chown=root:root . .

# Use the node user from the image (instead of the root user)
USER node

###################
# BUILD FOR PRODUCTION
###################

FROM node:lts-bullseye-slim As build

WORKDIR /usr/src/app

COPY --chown=root:root package*.json pnpm-lock.yaml .npmrc ./

# In order to run `npm run build` we need access to the Nest CLI which is a dev dependency. In the previous development stage we ran `npm ci` which installed all dependencies, so we can copy over the node_modules directory from the development image
COPY --chown=root:root --from=development /usr/src/app/node_modules ./node_modules

COPY --chown=root:root . .

# Run the build command which creates the production bundle
RUN npm run build

# Set NODE_ENV environment variable
ENV NODE_ENV production

USER root


# 生产环境构建
FROM node:lts-bullseye-slim As production

USER root
RUN cp /etc/apt/sources.list /etc/apt/sources.list.bak; \
  echo " " > /etc/apt/sources.list; \
  echo "deb http://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-updates main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.tuna.tsinghua.edu.cn/debian/ bullseye-backports main contrib non-free" >> /etc/apt/sources.list; \
  echo "deb http://mirrors.tuna.tsinghua.edu.cn/debian-security bullseye-security main contrib non-free" >> /etc/apt/sources.list; 

RUN apt-get update;
RUN apt-get install --assume-yes apt-utils;
RUN apt-get install curl -y
RUN apt-get install python3.10 -y
RUN apt-get install python3-pip -y
RUN pip3 config set global.index-url http://mirrors.aliyun.com/pypi/simple
RUN pip3 config set install.trusted-host mirrors.aliyun.com
# canvas -> https://github.com/Automattic/node-canvas/wiki/Installation%3A-Ubuntu-and-other-Debian-based-systems
RUN apt-get install build-essential libcairo2-dev libpango1.0-dev libjpeg-dev libgif-dev librsvg2-dev -y

# 中文字体安装
RUN apt-get install -y --no-install-recommends wget unzip fontconfig -y
RUN wget https://images.shubuzuo.top/devops/fontfile/NotoSansCJKsc-hinted.zip \
  && unzip NotoSansCJKsc-hinted.zip -d /usr/share/fonts/truetype/noto \
  && rm NotoSansCJKsc-hinted.zip
# RUN apt-get install fonts-arphic-ukai fonts-arphic-uming fonts-ipafont-mincho fonts-ipafont-gothic fonts-unfonts-core
RUN fc-cache -fv

WORKDIR /usr/src/app

COPY --chown=root:root package*.json pnpm-lock.yaml .npmrc ./

RUN npm i pnpm -g --registry=https://registry.npmmirror.com/
RUN pnpm i

# 源代码依赖的的相关静态资源
# 依赖包安装
COPY --chown=root:root --from=build /usr/src/app/bin ./bin
COPY --chown=root:root --from=build /usr/src/app/templates ./templates
COPY --chown=root:root --from=build /usr/src/app/public ./public
RUN cd bin && pip3 install -r requirements.txt
COPY --chown=root:root --from=build /usr/src/app/dist ./dist

# 健康检查

HEALTHCHECK --interval=2s --timeout=5s CMD curl -f http://localhost:5000/api/app/ping || exit 1
EXPOSE 5000

# 启动服务
CMD [ "node", "dist/main.js" ]
# docker build -t projectName:latest .
# docker run -v ./.env:/usr/src/app/.env projectName:0.0.1 
```
