# 此处记录详细 ubuntu 预装程序

- 代理工具 v2raya, 不使用 qv2ray
- edge (不知为何, 装完之后一直不能启动, 过了一阵子才成功启动)
- vscode-insider
- qq (比起微信, 跨平台了)
- git
- curl
- nvm 
- docker
- copyq
- dbeaver
- MongoDb Compass
- VLC
- Obs Studio
- 四叶草拼音输入
- calibre
- bing wall
- ulauncher


相关命令

```bash
snap install v2raya
sudo apt install git
sudo apt install curl # 小心从 snap 安装的 curl, 默认权限不够, 下载 node lts 时下载不下来
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash # https://github.com/nvm-sh/nvm?tab=readme-ov-file#troubleshooting-on-linux
nvm ls-remote
sudo snap install vlc
sudo apt install flatpak
flatpak install flathub com.obsproject.Studio
sudo apt install fcitx5-rime
flatpak install flathub com.calibre_ebook.calibre
```
