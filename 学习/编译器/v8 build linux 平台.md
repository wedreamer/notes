# v8 build linux 平台

## 为何有此文

昨天，想 build 一份 v8 的源码，其实是为了得到相关库，不管是动态库或者是静态库，然后使用该库搞明白 c++ 和 js 之间的互操作，即 js 调用 c++ 相关函数以及属性，c++ 调用 js 相关函数及属性。这个时候就需要得到 v8 相关库，大概两年前笔者也尝试过 v8 的相关调试，但是参考的是 IDE 相关部分，使用 vscode 跑示例 helloworld，那次是直接基于源码编译及 debugger，这次就不大一样。本次尝试 v8 build 遇到很多莫名其妙的问题，这些问题是文档中没有进行额外说明的，或者写文档的人认为大家都晓得的，其实并非如此，而是文档写的并不好，这就导致参考文档并不能顺利 build 出想要的产物，其他平台并不晓得具体情况如何，笔者自己环境确实遇到很多问题，比如生成的静态库无法链接，识别为未知架构，后来晓得应该使用 lld 进行链接就没问题了。但是还有自定义 c++ 库的问题，导致链接静态库不成功。之后经过反复尝试更改相关 build 参数，不使用 clang 以及自定义 c++ 库才得到一份预期的静态库。这些也是文档未曾提及的，实际应该说明，尤其是 build 参数之间的内隐关系。因此笔者认为有必要进行相关行文说明及总结，以供同问的人参考。

## 环境

```bash
uname -a
# Linux shubuzuo-system 6.8.0-52-generic #53-Ubuntu SMP PREEMPT_DYNAMIC Sat Jan 11 00:06:25 UTC 2025 x86_64 x86_64 x86_64 GNU/Linux
cat /etc/issue
# Ubuntu 24.04.1 LTS \n \l
clang -v
# Ubuntu clang version 18.1.3 (1ubuntu1)
# Target: x86_64-pc-linux-gnu
# Thread model: posix
# InstalledDir: /usr/bin
# Found candidate GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/13
# Selected GCC installation: /usr/bin/../lib/gcc/x86_64-linux-gnu/13
# Candidate multilib: .;@m64
# Candidate multilib: 32;@m32
# Candidate multilib: x32;@mx32
# Selected multilib: .;@m64
gcc -v
# Using built-in specs.
# COLLECT_GCC=gcc
# COLLECT_LTO_WRAPPER=/usr/libexec/gcc/x86_64-linux-gnu/13/lto-wrapper
# OFFLOAD_TARGET_NAMES=nvptx-none:amdgcn-amdhsa
# OFFLOAD_TARGET_DEFAULT=1
# Target: x86_64-linux-gnu
# Configured with: ../src/configure -v --with-pkgversion='Ubuntu 13.3.0-6ubuntu2~24.04' --with-bugurl=file:///usr/share/doc/gcc-13/README.Bugs --enable-languages=c,ada,c++,go,d,fortran,objc,obj-c++,m2 --prefix=/usr --with-gcc-major-version-only --program-suffix=-13 --program-prefix=x86_64-linux-gnu- --enable-shared --enable-linker-build-id --libexecdir=/usr/libexec --without-included-gettext --enable-threads=posix --libdir=/usr/lib --enable-nls --enable-bootstrap --enable-clocale=gnu --enable-libstdcxx-debug --enable-libstdcxx-time=yes --with-default-libstdcxx-abi=new --enable-libstdcxx-backtrace --enable-gnu-unique-object --disable-vtable-verify --enable-plugin --enable-default-pie --with-system-zlib --enable-libphobos-checking=release --with-target-system-zlib=auto --enable-objc-gc=auto --enable-multiarch --disable-werror --enable-cet --with-arch-32=i686 --with-abi=m64 --with-multilib-list=m32,m64,mx32 --enable-multilib --with-tune=generic --enable-offload-targets=nvptx-none=/build/gcc-13-fG75Ri/gcc-13-13.3.0/debian/tmp-nvptx/usr,amdgcn-amdhsa=/build/gcc-13-fG75Ri/gcc-13-13.3.0/debian/tmp-gcn/usr --enable-offload-defaulted --without-cuda-driver --enable-checking=release --build=x86_64-linux-gnu --host=x86_64-linux-gnu --target=x86_64-linux-gnu --with-build-config=bootstrap-lto-lean --enable-link-serialization=2
# Thread model: posix
# Supported LTO compression algorithms: zlib zstd
# gcc version 13.3.0 (Ubuntu 13.3.0-6ubuntu2~24.04) 
```

## 开始的试错

[Building V8 from source](https://v8.dev/docs/build)

[Checking out the V8 source code](https://v8.dev/docs/source-code)

[Building V8 with GN](https://v8.dev/docs/build-gn)

以下展示相关主要命令，详情可参考上述参考文档。

```bash
# depot_tools 工具
git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
export PATH=/path/to/depot_tools:$PATH
mkdir ~/v8
cd ~/v8
# 拉代码
fetch v8
cd v8
# 拉相关代码及依赖库
git pull && gclient sync
# 查看最近打的一个 tag, 一般情况下就是最新发布的版本
git tag --sort=-creatordate | head -n 1
# 13.5.119 # 预期笔者当前最新的发布版本
# build 会默认生成一份 args.gn 该文件用于后续 build 相关参数配置, build 工具为 ninja
tools/dev/gm.py x64.release
```

笔者环境下默认生成的 `args.gn`，如下：

```bash
# 执行该语句将会自动生成，默认位置为 out/x64.release/args.gn
tools/dev/gm.py x64.release
# # mkdir -p out/x64.release
# # echo > out/x64.release/args.gn << EOF
# is_component_build = false
# is_debug = false
# target_cpu = "x64"
# v8_enable_sandbox = true
# v8_enable_backtrace = true
# v8_enable_disassembler = true
# v8_enable_object_print = true
# v8_enable_verify_heap = true
# dcheck_always_on = false
# EOF
# # gn gen out/x64.release
```

```env
is_component_build = false
is_debug = false
target_cpu = "x64"
v8_enable_sandbox = true
v8_enable_backtrace = true
v8_enable_disassembler = true
v8_enable_object_print = true
v8_enable_verify_heap = true
dcheck_always_on = false
```

```bash
# 默认编译出来的相关库
cd out/x64.release/obj
ls *.a # 静态库 因为 is_component_build = false
# libv8_libbase.a  libv8_libplatform.a
```

使用时尤其本地测试时，可能只想引用一个库，而不是单个单个引用，这个时候就需要把产物全部打包到一个静态库中，有时候又想要动态库一个一个导入，该如何做呢？

将 `is_component_build` 设置为 true 可以生成几个模块的动态库。更改完成需要重新生成 build 相关文件。

```env
is_component_build = true
is_debug = false
target_cpu = "x64"
v8_enable_sandbox = true
v8_enable_backtrace = true
v8_enable_disassembler = true
v8_enable_object_print = true
v8_enable_verify_heap = true
dcheck_always_on = false
```

```bash
cd out/x64.release
# gn 参考 https://v8.dev/docs/build-gn
gn gen .
# 重新 build
tools/dev/gm.py x64.release
```

单个静态库生成则更改为如下：

```env
is_component_build = false
is_debug = false
target_cpu = "x64"
v8_enable_sandbox = true
v8_enable_backtrace = true
v8_enable_disassembler = true
v8_enable_object_print = true
v8_enable_verify_heap = true
dcheck_always_on = false
v8_monolithic = true
v8_use_external_startup_data = false
```

```bash
cd out/x64.release
# gn 参考 https://v8.dev/docs/build-gn
gn gen .
# 重新 build
ninja -C out/x64.release v8_monolith
```

以上命令 build 出来的相关库会有一些问题，首先是链接的时候库未知架构问题，其次是由于 v8 使用 c++ 自定义标准库导致链接时不能识别标准 c++ 及 v8 函数， v8 函数确实在库中是有的，最终导致链接失败。

### 静态库未知架构

使用 `v8_monolith` 或者 `v8_libbase` 与 `v8_libplatform` 进行 build，代码为 `hello world` 示例代码，出现问题 `unknown architecture of input file /tmp/v8/v8/out/x64.release/obj/libv8_monolith.a(api.o) is incompatible with i386:x86-64 output`，截图如下:

![unknown architecture of input file](https://images.shubuzuo.top/2025-02/2025-02-19/pic_1739951156957-13.png)  

后面发现文档 [Getting started with embedding V8](https://v8.dev/docs/embed) 中的运行语句为:

```bash
g++ -I. -Iinclude samples/hello-world.cc -o hello_world -fno-rtti -fuse-ld=lld -lv8_monolith -lv8_libbase -lv8_libplatform -ldl -Lout.gn/x64.release.sample/obj/ -pthread -std=c++20 -DV8_COMPRESS_POINTERS -DV8_ENABLE_SANDBOX
```

遂怀疑是链接的问题，使用 `lld` 进行链接则避免该问题。

### 自定义 c++ 标准库

上述问题解决之后，依旧会出现其他问题，截图如下：

![Undefined reference to v8::platform::NewDefaultPlatform](https://images.shubuzuo.top/2025-02/2025-02-19/pic_1739951388042-2.png)  

有前人几年前遇到了这个问题，相关讨论地址 [Undefined reference to v8::platform::NewDefaultPlatform](https://groups.google.com/g/v8-users/c/Jb1VSouy2Z0)。

## 后续的尝试

```env
target_cpu = "x64"
target_os = "linux"
is_debug = true
is_component_build = false
v8_monolithic = true
v8_static_library = false
v8_use_external_startup_data = false
use_custom_libcxx = false
is_clang = false
```

```bash
cd out/x64.release
# gn 参考 https://v8.dev/docs/build-gn
gn gen .
# 重新 build
ninja -C out/x64.release v8_monolith
```

这个是很 ok 的，`hello world` 可以编译及链接，正常运行及调试（hello world 程序内调试）。

## 反思

以上问题产生的原因及解决很大程度归于文档方面，为何？如果文档能说明构建使用的链接器就能避免问题 1 的产生，而第二个问题的产生完全就是没有相关信息参考的结果，对于 v8 相关的 `args.gn` 说明是很有必要的，虽然最后能得到预期的产物，但是这个途径是瞎摸索的，相反如果给出的信息比较详细就有所参考，这个问题对于 v8 的维护者来说是很轻松就能解决的，因为后续的编码及调试都需要这个最基础的步骤。而对于 [Undefined reference to v8::platform::NewDefaultPlatform](https://groups.google.com/g/v8-users/c/Jb1VSouy2Z0) 问题的解决，最后也是提出者自己尝试到了合适的配置的结果。但是对于该交流，尤其是几年前已经出现同时体现出了揭示 `v8 args.gn` 的必要性，似乎 `v8` 团队并没有意识到该问题，或者没有做相关改进。这也警惕着我们在写文档时需要慎重，文档所带来的应该是明灯指路的作用，而不是更多的黑暗，这样会导致使用者更多的迷惑，或者花费巨大的成本和精力去思索答案。
