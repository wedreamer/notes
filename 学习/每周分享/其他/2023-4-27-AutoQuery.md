# AutoQuery

## 需要安装 

```bash
dotnet tool install --global x
```

## 初始化项目

```bash
x new web Chinook
cd Chinook
x mix autocrudgen sqlite chinook.sqlite
# 改变 Configure.Db.cs

```

## 部署

```bash
# 服务器安装 docker, docker compose
# 生成 nginx 部署脚本
x mix build release-ghr-vanilla
# 上传到目标服务器执行
scp ./.deploy/nginx-proxy-compose.yml tencentCloud:~/nginx/nginx-proxy-compose.yml
cd ~/nginx
docker compose -f ./nginx-proxy-compose.yml up -d
# github action 相关变量
DEPLOY_HOST chinook.shubuzuo.top
DEPLOY_PORT 22
DEPLOY_USERNAME ubuntu
DEPLOY_KEY 
LETSENCRYPT_EMAIL shubuzuo@gmail.com
# 将当前用户添加到 docker 和 docker compose 中去
sudo usermod -aG docker $USER
sudo systemctl restart docker
```

## 附加: docker, docker compose 在 ubuntu 22.04 LTS 中的安装 [docker 安装](https://docs.docker.com/engine/install/ubuntu/)

您可以按照以下步骤在 Ubuntu 中安装 Docker 和 Docker Compose：

1. 更新您的 Ubuntu 系统。打开终端，依次运行以下命令：

```bash
sudo apt update
sudo apt upgrade
sudo apt full-upgrade
```

2. 添加 Docker 库。首先，安装必要的证书并允许 apt 包管理器使用以下命令通过 HTTPS 使用存储库：
```bash
sudo apt install apt-transport-https ca-certificates curl software-properties-common gnupg lsb-release
```
然后，运行以下命令添加 Docker 的官方 GPG 密钥：
```bash
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
```
添加 Docker 官方库：
```bash
echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
```
使用命令更新 Ubuntu 源列表：
```bash
sudo apt update
```

3. 安装 Docker。最后，运行以下命令在 Ubuntu 22.04 LTS 服务器中安装最新 Docker CE：
```bash
sudo apt install docker-ce docker-ce-cli containerd.io docker-compose-plugin
```

您可以在这篇文章中找到更多详细信息¹。希望这些信息对您有所帮助！

Source: Conversation with Bing, 2023/7/3
(1) 如何在 Ubuntu 22.04 LTS 中安装 Docker 和 Docker Compose. https://linux.cn/article-14871-1.html.
(2) 如何在 Ubuntu 20.04 上安装和使用 Docker Compose - 知乎. https://zhuanlan.zhihu.com/p/144449564.
(3) 如何在 Ubuntu 22.04 LTS 中安装 Docker 和 Docker Compose .... https://www.51cto.com/article/715086.html.
(4) 如何在 Ubuntu 22.04 LTS 中安装 Docker 和 Docker Compose .... https://zhuanlan.zhihu.com/p/547169542.
