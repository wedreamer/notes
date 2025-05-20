# 此处记录详细 ubuntu 预装程序

> 昨天晚上安装的 ubuntu 24.04, 这里记录要安装的必要软件, 后续发现需要继续安装则进行补充

- 代理工具 v2raya, 不使用 qv2ray
- edge (不知为何, 装完之后一直不能启动, 过了一阵子才成功启动)
- vscode-insider
- qq
- weixin
- dingding
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
- 火焰截图
- z-lib
- fsearch

相关命令

```bash
# 使用 GUI.for.Clash-linux-amd64GUI.for.Clash-linux-amd64
# https://github.com/GUI-for-Cores/GUI.for.Clash
snap install v2raya
sudo apt install git
sudo apt install curl # 小心从 snap 安装的 curl, 默认权限不够, 下载 node lts 时下载不下来
curl -o- https://raw.githubusercontent.com/nvm-sh/nvm/v0.39.7/install.sh | bash # https://github.com/nvm-sh/nvm?tab=readme-ov-file#troubleshooting-on-linux
nvm ls-remote
# 不使用 copyq 使用基于 gnome 的粘贴版工具
# pano
# sudo apt install gir1.2-gda-5.0 gir1.2-gsound-1.0
# git clone https://github.com/oae/gnome-shell-pano.git
# cd ./gnome-shell-pano
# yarn install
# yarn build
# ln -s "$PWD/dist" "$HOME/.local/share/gnome-shell/extensions/pano@elhan.io"
copyq
sudo snap install vlc
sudo apt install flatpak
flatpak install flathub com.obsproject.Studio
sudo apt install fcitx5-rime
flatpak install flathub com.calibre_ebook.calibre
```
