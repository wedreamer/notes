# readFile 为例 nodejs 如何实现

## 环境及代码

```bash
git clone git@github.com:nodejs/node.git
git checkout b07ff551db152ccb507e71160ce077e235206b16
```

## 目的

以 `fs.readFile` 函数为例, 看看 nodejs 如何实现, 借此了解 nodejs 相关的底层实现。

## fs.readFileSync

`fs.readFileSync` js 相关文件位置在 `lib/fs.js:438`

### js 相关实现

```js
/**
 * Synchronously reads the entire contents of a file.
 * @param {string | Buffer | URL | number} path
 * @param {{
 *   encoding?: string | null;
 *   flag?: string;
 *   }} [options]
 * @returns {string | Buffer}
 */
function readFileSync(path, options) {
  // 读所以设置为 r flag
  options = getOptions(options, { flag: 'r' });

  if (options.encoding === 'utf8' || options.encoding === 'utf-8') {
    if (!isInt32(path)) {
      path = getValidatedPath(path);
    }
    return binding.readFileUtf8(path, stringToFlags(options.flag));
  }

  const isUserFd = isFd(path); // File descriptor ownership
  const fd = isUserFd ? path : fs.openSync(path, options.flag, 0o666);

  const stats = tryStatSync(fd, isUserFd);
  const size = isFileType(stats, S_IFREG) ? stats[8] : 0;
  let pos = 0;
  let buffer; // Single buffer with file data
  let buffers; // List for when size is unknown

  if (size === 0) {
    buffers = [];
  } else {
    buffer = tryCreateBuffer(size, fd, isUserFd);
  }

  let bytesRead;

  if (size !== 0) {
    do {
      bytesRead = tryReadSync(fd, isUserFd, buffer, pos, size - pos);
      pos += bytesRead;
    } while (bytesRead !== 0 && pos < size);
  } else {
    do {
      // The kernel lies about many files.
      // Go ahead and try to read some bytes.
      buffer = Buffer.allocUnsafe(8192);
      bytesRead = tryReadSync(fd, isUserFd, buffer, 0, 8192);
      if (bytesRead !== 0) {
        ArrayPrototypePush(buffers, buffer.slice(0, bytesRead));
      }
      pos += bytesRead;
    } while (bytesRead !== 0);
  }

  if (!isUserFd)
    fs.closeSync(fd);

  if (size === 0) {
    // Data was collected into the buffers list.
    buffer = Buffer.concat(buffers, pos);
  } else if (pos < size) {
    buffer = buffer.slice(0, pos);
  }

  if (options.encoding) buffer = buffer.toString(options.encoding);
  return buffer;
}
```

如上可以看到, 如果为 `options.encoding` 为 `utf8` 的话, 会直接去调用 `binding.readFileUtf8` 函数, 这个函数最终是由 `c++` 实现的, 下面看下内部模块是如何被引用的, 也就是 `binding` 从何而来。本文件相关代码摘取如下:

```js
// lib/fs.js:66
const binding = internalBinding('fs');
```

现在的问题就是 `internalBinding` 从何而来, 为何该 `js` 文件中没有该函数声明却能调用, 同样的函数如 `require` 也是如此, 这些涉及到 `c++` 代码对 `v8` 中 `js` 的一些功能接口实现,  同时也关系到 `node` 启动过程的模块加载, 此处暂不做过多讨论, 后续会有相关文章讨论。

现在需要关注的是 `binding.readFileUtf8`, 该函数是如何实现的.

#### binding.readFileUtf8 相关实现

关系到 `binding.readFileUtf8` 的实现, 我们猜测为内部 `c++` 实现的函数, 实际上确实如此, 怎么确定呢? 暂不做讨论, 定位到相关函数位置 `src/node_file.cc:2651`, 该文件中有代码 `SetMethod(isolate, target, "readFileUtf8", ReadFileUtf8);` 该函数即证明暴露给 `js` 的函数名为 `readFileUtf8`, 原函数名为 `ReadFileUtf8`. 至于该代码的详细调用时机以及相关信息后行文再讨论.

```c++
// src/node_file.cc:2651
// 该函数 c++ 实现，被 js 调用，v8 针对每一个回调都会有相关回调信息
static void ReadFileUtf8(const FunctionCallbackInfo<Value>& args) {
  // Environment nodejs 封装环境信息的类, 后行文再讨论
  Environment* env = Environment::GetCurrent(args);
  // 当前 v8 的隔离容器
  auto isolate = env->isolate();

  CHECK_GE(args.Length(), 2);

  CHECK(args[1]->IsInt32());
  const int flags = args[1].As<Int32>()->Value();

  // 后续实现实际是通过 libuv 实现的
  // uv 文件描述符及 uv 文件状态
  uv_file file;
  uv_fs_t req;

  // 判断是否为文件描述符
  bool is_fd = args[0]->IsInt32();

  // Check for file descriptor
  if (is_fd) {
    file = args[0].As<Int32>()->Value();
  } else {
    BufferValue path(env->isolate(), args[0]);
    CHECK_NOT_NULL(*path);
    // 文件路径转换为 namespace 路径
    ToNamespacedPath(env, &path);
    // 文件权限检查
    if (CheckOpenPermissions(env, path, flags).IsNothing()) return;

    // 提供给程序监控用的相关信息
    FS_SYNC_TRACE_BEGIN(open);
    // 通过 uv_fs_open 打开文件 
    file = uv_fs_open(nullptr, &req, *path, flags, 0666, nullptr);
    FS_SYNC_TRACE_END(open);
    // 判断是否成功打开
    if (req.result < 0) {
      uv_fs_req_cleanup(&req);
      // req will be cleaned up by scope leave.
      return env->ThrowUVException(
          static_cast<int>(req.result), "open", nullptr, path.out());
    }
  }

  // 这里定义了该函数作用域离开时执行的相关操作
  auto defer_close = OnScopeLeave([file, is_fd, &req]() {
    if (!is_fd) {
      FS_SYNC_TRACE_BEGIN(close);
      CHECK_EQ(0, uv_fs_close(nullptr, &req, file, nullptr));
      FS_SYNC_TRACE_END(close);
    }
    // 释放 req
    uv_fs_req_cleanup(&req);
  });

  // 文件内容
  std::string result{};
  // 文件内容缓冲区
  char buffer[8192];
  // 初始化为 uv buffer
  uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));

  FS_SYNC_TRACE_BEGIN(read);
  while (true) {
    // 直接读直到读完
    auto r = uv_fs_read(nullptr, &req, file, &buf, 1, -1, nullptr);
    if (req.result < 0) {
      FS_SYNC_TRACE_END(read);
      // req will be cleaned up by scope leave.
      return env->ThrowUVException(
          static_cast<int>(req.result), "read", nullptr);
    }
    if (r <= 0) {
      break;
    }
    result.append(buf.base, r);
  }
  FS_SYNC_TRACE_END(read);

  // 转换为 v8 内部值供 js 访问
  Local<Value> val;
  if (!ToV8Value(env->context(), result, isolate).ToLocal(&val)) {
    return;
  }

  // 设置 js 回调函数返回值
  args.GetReturnValue().Set(val);
}
```

需要注意的是 `uv_fs_read` 这个方法是到底是同步还是异步的呢? 其实这个方法也可以同步也可以异步, 后续会有相关说明, 具体代码可以看 `node` 依赖的 `libuv` 代码, 为什么调用该方法的 `fs.readFileSync` 却是同步的呢? 这里就设计到同步异步的场景，对于此处调用该方法的代码, 他在调用该方法之后要收到结果只能一直等待直到结果返回, 这就是同步而且阻塞, 但是这个方法如果异步调用时, 对于事件循环来说却是异步的, 对于建设在事件循环之上的 `node` 平台更是异步, 因为其调用之后如果文件读完会在之后的事件循环中得到该结果. 如果此处更改为异步该如何做, 后面我们可以看一下 `fs.read` 中异步是如何实现的.

#### tryReadSync

对于 `fs.readFileSync` 剩下的代码则将读取数据的逻辑实现在 `js` 层中, 但主要方法还是在 `c++` 层中实现, `fs.openSync` 底层调用 `Open`, `tryStatSync` 为 `FStat`, `fs.closeSync` 为 `Close`, `tryCreateBuffer` 为 `Uint8Array`(v8), `tryReadSync` 为 `Read`.

下面我们来看一下 `Read` 函数实现, 相关代码及注释如下:

```c++
// src/node_file.cc:2587
/*
 * Wrapper for read(2).
 *
 * bytesRead = fs.read(fd, buffer, offset, length, position)
 *
 * 0 fd        int32. file descriptor
 * 1 buffer    instance of Buffer
 * 2 offset    int64. offset to start reading into inside buffer
 * 3 length    int32. length to read
 * 4 position  int64. file position - -1 for current position
 */
// js 调用 c++ v8 回调信息
static void Read(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  const int argc = args.Length();
  CHECK_GE(argc, 5);

  CHECK(args[0]->IsInt32());
  const int fd = args[0].As<Int32>()->Value();

  CHECK(Buffer::HasInstance(args[1]));
  // 转换为本地对象
  Local<Object> buffer_obj = args[1].As<Object>();
  char* buffer_data = Buffer::Data(buffer_obj);
  size_t buffer_length = Buffer::Length(buffer_obj);

  CHECK(IsSafeJsInt(args[2]));
  const int64_t off_64 = args[2].As<Integer>()->Value();
  CHECK_GE(off_64, 0);
  CHECK_LT(static_cast<uint64_t>(off_64), buffer_length);
  const size_t off = static_cast<size_t>(off_64);

  CHECK(args[3]->IsInt32());
  const size_t len = static_cast<size_t>(args[3].As<Int32>()->Value());
  CHECK(Buffer::IsWithinBounds(off, len, buffer_length));

  CHECK(IsSafeJsInt(args[4]) || args[4]->IsBigInt());
  const int64_t pos = args[4]->IsNumber() ?
                      args[4].As<Integer>()->Value() :
                      args[4].As<BigInt>()->Int64Value();

  char* buf = buffer_data + off;
  uv_buf_t uvbuf = uv_buf_init(buf, len);

  if (argc > 5) {  // read(fd, buffer, offset, len, pos, req)
    // 异步调用
    FSReqBase* req_wrap_async = GetReqWrap(args, 5);
    CHECK_NOT_NULL(req_wrap_async);
    FS_ASYNC_TRACE_BEGIN0(UV_FS_READ, req_wrap_async)
    AsyncCall(env, req_wrap_async, args, "read", UTF8, AfterInteger,
              uv_fs_read, fd, &uvbuf, 1, pos);
  } else {  // read(fd, buffer, offset, len, pos)
    // 同步调用
    FSReqWrapSync req_wrap_sync("read");
    FS_SYNC_TRACE_BEGIN(read);
    const int bytesRead = SyncCallAndThrowOnError(
        env, &req_wrap_sync, uv_fs_read, fd, &uvbuf, 1, pos);
    FS_SYNC_TRACE_END(read, "bytesRead", bytesRead);

    if (is_uv_error(bytesRead)) {
      return;
    }

    args.GetReturnValue().Set(bytesRead);
  }
}
```

通过以上代码我们可以发现不管是同步调用还是异步调用, 都是通过 `Read` 函数实现的, 而同步异步之间调用的差别在于 `FSReqBase` 和 `FSReqWrapSync`, `AsyncCall` 和 `SyncCallAndThrowOnError`. 同步之后因为能立即执行可以根据返回值判断是否出错, 背后也是通过 `libuv` 来完成文件调用的, 相对的因为异步调用不是立即执行, 因此额外会有调用的一些参数及上下文信息以及回调, 这个回调很重要. 异步调用的实现我们后续会讨论, 现在看一下同步调用的 `FSReqWrapSync` 与 `SyncCallAndThrowOnError`.

##### FSReqWrapSync

```c++
// src/node_file.h:458
class FSReqWrapSync {
 public:
  FSReqWrapSync(const char* syscall = nullptr,
                const char* path = nullptr,
                const char* dest = nullptr)
      : syscall_p(syscall), path_p(path), dest_p(dest) {}
  ~FSReqWrapSync() { uv_fs_req_cleanup(&req); }

  uv_fs_t req;
  const char* syscall_p;
  const char* path_p;
  const char* dest_p;

  FSReqWrapSync(const FSReqWrapSync&) = delete;
  FSReqWrapSync& operator=(const FSReqWrapSync&) = delete;

  // TODO(joyeecheung): move these out of FSReqWrapSync and into a special
  // class for mkdirp
  FSContinuationData* continuation_data() const {
    return continuation_data_.get();
  }
  void set_continuation_data(std::unique_ptr<FSContinuationData> data) {
    continuation_data_ = std::move(data);
  }

 private:
  std::unique_ptr<FSContinuationData> continuation_data_;
};
```

可以看到实现相对来说比较简单, 而对于上文 `Read` 函数提到的代码 `FSReqWrapSync req_wrap_sync("read")` 实例无非是携带了 `read` 作为 `syscall_p` 的信息, 下面看一下 `SyncCallAndThrowOnError`.

##### SyncCallAndThrowOnError

`SyncCallAndThrowOnError` 的实现实际是调用了 `SyncCallAndThrowIf`:

```c++
// src/node_file-inl.h:389
// Similar to SyncCall but throws immediately if there is an error.
template <typename Func, typename... Args>
int SyncCallAndThrowOnError(Environment* env,
                            FSReqWrapSync* req_wrap,
                            Func fn,
                            Args... args) {
  return SyncCallAndThrowIf(is_uv_error, env, req_wrap, fn, args...);
}
```

看一下 `SyncCallAndThrowIf`:

```c++
// src/node_file-inl.h:366
// Similar to SyncCall but throws immediately if there is an error.
template <typename Predicate, typename Func, typename... Args>
int SyncCallAndThrowIf(Predicate should_throw,
                       Environment* env,
                       FSReqWrapSync* req_wrap,
                       Func fn,
                       Args... args) {
  env->PrintSyncTrace();
  int result = fn(nullptr, &(req_wrap->req), args..., nullptr);
  if (should_throw(result)) {
    env->ThrowUVException(result,
                          req_wrap->syscall_p,
                          nullptr,
                          req_wrap->path_p,
                          req_wrap->dest_p);
  }
  return result;
}
```

到这里我们结合上文 `Read` 函数调用的细节, 就知道 `fn(nullptr, &(req_wrap->req), args..., nullptr)` 最终有效调用为 `uv_fs_read(nullptr, &(req_wrap->req), fd, &uvbuf, 1, pos, nullptr)`. 可以看下 `libuv` 中 `uv_fs_read` 的函数参考:

```c++
int uv_fs_read(uv_loop_t* loop, uv_fs_t* req, uv_file file, const uv_buf_t bufs[], unsigned int nbufs, int64_t offset, uv_fs_cb cb)
```

对应一下函数, 我们很容易知道 `FSReqWrapSync` 实例中 `req` 字段类型即为 `uv_fs_t`. 但是我们可能会疑惑 `loop` 为什么可以为 `nullptr`, 这是因为 `libuv` 中 `uv_fs_read` 函数实现时如果 `cb` 为空的话，就会使用 `uv__fs_read` 直接进行系统调用, 相关代码不再进行展示, 读者可以自行查阅, 参考信息如下 `deps/uv/src/unix/fs.c:2032` -> `deps/uv/src/unix/fs.c:139` -> `deps/uv/src/unix/fs.c:1695` -> `deps/uv/src/unix/fs.c:516`.

至此我们基本走了一遍 `fs.readFileSync` 的实现, 下面我们看一下 `fs.read` 的实现, 以及异步调用的实现.

## fs.read

`fs.read` js 相关文件位置在 `lib/fs.js:352`. 下面对其相关代码增加注释:

```js
// lib/fs.js:352
/**
 * Asynchronously reads the entire contents of a file.
 * @param {string | Buffer | URL | number} path
 * @param {{
 *   encoding?: string | null;
 *   flag?: string;
 *   signal?: AbortSignal;
 *   } | string} [options]
 * @param {(
 *   err?: Error,
 *   data?: string | Buffer
 *   ) => any} callback
 * @returns {void}
 */
function readFile(path, options, callback) {
  // 可能是三个或者两个参数
  callback ||= options;
  // 校验
  validateFunction(callback, 'cb');
  // 读所以设置为 r flag
  options = getOptions(options, { flag: 'r' });
  // js 实现的类 之后讨论
  ReadFileContext ??= require('internal/fs/read/context');
  // 初始化上下文
  const context = new ReadFileContext(callback, options.encoding);
  // 可能是文件描述符
  context.isUserFd = isFd(path); // File descriptor ownership

  if (options.signal) {
    context.signal = options.signal;
  }
  // 如果是文件描述符的话, 同步函数执行完成之后会调用 readFileAfterOpen
  if (context.isUserFd) {
    process.nextTick(function tick(context) {
      ReflectApply(readFileAfterOpen, { context }, [null, path]);
    }, context);
    return;
  }

  // 检查是否中断
  if (checkAborted(options.signal, callback))
    return;

  const flagsNumber = stringToFlags(options.flag, 'options.flag');
  // FSReqCallback 初始化
  const req = new FSReqCallback();
  req.context = context;
  req.oncomplete = readFileAfterOpen;
  // 调用内部 c++ 函数, 传递相关信息
  binding.open(getValidatedPath(path), flagsNumber, 0o666, req);
}
```

### ReadFileContext

看下 `ReadFileContext` 相关实现:

```js
// lib/internal/fs/read/context.js:71
class ReadFileContext {
  constructor(callback, encoding) {
    this.fd = undefined;
    this.isUserFd = undefined;
    this.size = 0;
    this.callback = callback;
    this.buffers = null;
    this.buffer = null;
    this.pos = 0;
    this.encoding = encoding;
    this.err = null;
    this.signal = undefined;
  }

  read() {
    let buffer;
    let offset;
    let length;

    // 检查是否中断
    if (this.signal?.aborted) {
      return this.close(
        new AbortError(undefined, { cause: this.signal.reason }));
    }
    if (this.size === 0) {
      // 第一次数据读取 buffer 初始化
      // 位置 lib/internal/fs/utils.js:141
      // const kReadFileUnknownBufferLength = 64 * 1024;
      buffer = Buffer.allocUnsafeSlow(kReadFileUnknownBufferLength);
      offset = 0;
      length = kReadFileUnknownBufferLength;
      this.buffer = buffer;
    } else {
      buffer = this.buffer;
      offset = this.pos;
      // 位置 lib/internal/fs/utils.js:142
      // const kReadFileBufferLength = 512 * 1024;
      length = MathMin(kReadFileBufferLength, this.size - this.pos);
    }

    // FSReqCallback 初始化
    const req = new FSReqCallback();
    // 绑定相关函数
    req.oncomplete = readFileAfterRead;
    req.context = this;

    // 调用内部 c++ 函数, 传递相关信息
    // 还是调用 Open 函数, 但是走的异步调用
    read(this.fd, buffer, offset, length, -1, req);
  }

  close(err) {
    if (this.isUserFd) {
      process.nextTick(function tick(context) {
        ReflectApply(readFileAfterClose, { context }, [null]);
      }, this);
      return;
    }

    // FSReqCallback 初始化
    const req = new FSReqCallback();
    req.oncomplete = readFileAfterClose;
    req.context = this;
    this.err = err;

    // 调用内部 c++ 函数, 传递相关信息
    // 还是调用 Close 函数, 但是走的异步调用
    close(this.fd, req);
  }
}
```

由上我们可以知道, `ReadFileContext` 是对读和关闭的封装, 实例化之后可以通过 `read` 和 `close` 进行相关操作, 而且都是异步的. 下面看一下 `readFileAfterOpen` 这个函数.

### readFileAfterOpen

相关实现位置在 `lib/fs.js:298`, 代码如下:

```js
// lib/fs.js:298
function readFileAfterOpen(err, fd) {
  // 调用者为 ReadFileContext
  const context = this.context;

  // 错误检查
  if (err) {
    context.callback(err);
    return;
  }

  context.fd = fd;

  // 初始化 FSReqCallback 实例
  const req = new FSReqCallback();
  // 设置下一步事件
  req.oncomplete = readFileAfterStat;
  req.context = context;
  // 调用 c++ 函数, 且为异步调用
  binding.fstat(fd, false, req);
}
```

由以上我们可以大致看出, 最开始调用 `open`, `open` 好了之后调用 `req.oncomplete`, 也就是 `readFileAfterOpen`, 之后如果成功则调用 `fstat` 函数, 当然这次又是新的 `FSReqCallback` 实例, `fstat` 好了之后调用 `readFileAfterStat`. 如果成功, 估计就能读了, 实际正如我们猜测的一样. 看下 `readFileAfterStat` 的实现, 如下:

```js
// lib/fs.js:314
function readFileAfterStat(err, stats) {
  const context = this.context;

  if (err)
    return context.close(err);

  // TODO(BridgeAR): Check if allocating a smaller chunk is better performance
  // wise, similar to the promise based version (less peak memory and chunked
  // stringify operations vs multiple C++/JS boundary crossings).
  const size = context.size = isFileType(stats, S_IFREG) ? stats[8] : 0;

  if (size > kIoMaxLength) {
    err = new ERR_FS_FILE_TOO_LARGE(size);
    return context.close(err);
  }

  try {
    if (size === 0) {
      // TODO(BridgeAR): If an encoding is set, use the StringDecoder to concat
      // the result and reuse the buffer instead of allocating a new one.
      context.buffers = [];
    } else {
      context.buffer = Buffer.allocUnsafeSlow(size);
    }
  } catch (err) {
    return context.close(err);
  }
  context.read();
}
```

与我们预计的基本没啥问题, 成功则读文件, 失败则关闭文件. 下面我们看下 `FSReqCallback` 的实现.

### FSReqCallback

`FSReqCallback` js 文件中定义来自内置 `fs` 模块, 即 `c++` 相关实现, 代码如下:

```js
// lib/fs.js:66
const binding = internalBinding('fs');

// lib/fs.js:83
const {
  FSReqCallback,
  statValues,
} = binding;
```

下面看下 `c++` 中的 `FSReqCallback`:

```c++
// src/node_file-inl.h:77
FSReqCallback::FSReqCallback(BindingData* binding_data,
                             v8::Local<v8::Object> req,
                             bool use_bigint)
  : FSReqBase(binding_data,
              req,
              AsyncWrap::PROVIDER_FSREQCALLBACK,
              use_bigint) {}
```

那 `FSReqBase` 又是如何呢? 一路追踪相关 `基类`, 路线为 `FSReqCallback` -> `FSReqBase` -> `ReqWrap<uv_fs_t>` -> `AsyncWrap` 和 `ReqWrapBase`, `ReqWrapBase` 没有基类, 继续追 `AsyncWrap` -> `BaseObject`.

现在我们看下上述 `读文件` 相关代码中主要使用了 `FSReqCallback` 实例哪写方法, 同时通过上述分析我们知道 `read` 操作之前可能会调用 `open` 和 `fstat` 函数, 我们推测 `open` 和 `fstat` 函数实现与 `fs.readFileSync` 中的 `Open` 和 `FStat` 函数实现应该类似, 尤其是关于 `FSReqCallback` 的相关使用. 下面我们看一下 `open` 函数的实现.

### Open

相关代码如下, 位置在 `src/node_file.cc:2581`:

```c++
// src/node_file.cc:2581
static void Open(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  const int argc = args.Length();
  CHECK_GE(argc, 3);

  BufferValue path(env->isolate(), args[0]);
  CHECK_NOT_NULL(*path);
  ToNamespacedPath(env, &path);

  CHECK(args[1]->IsInt32());
  const int flags = args[1].As<Int32>()->Value();

  CHECK(args[2]->IsInt32());
  const int mode = args[2].As<Int32>()->Value();

  if (argc > 3) {  // open(path, flags, mode, req)
    FSReqBase* req_wrap_async = GetReqWrap(args, 3);
    CHECK_NOT_NULL(req_wrap_async);
    if (AsyncCheckOpenPermissions(env, req_wrap_async, path, flags).IsNothing())
      return;
    req_wrap_async->set_is_plain_open(true);
    FS_ASYNC_TRACE_BEGIN1(
        UV_FS_OPEN, req_wrap_async, "path", TRACE_STR_COPY(*path))
    AsyncCall(env, req_wrap_async, args, "open", UTF8, AfterInteger,
              uv_fs_open, *path, flags, mode);
  } else {  // open(path, flags, mode)
    if (CheckOpenPermissions(env, path, flags).IsNothing()) return;
    FSReqWrapSync req_wrap_sync("open", *path);
    FS_SYNC_TRACE_BEGIN(open);
    int result = SyncCallAndThrowOnError(
        env, &req_wrap_sync, uv_fs_open, *path, flags, mode);
    FS_SYNC_TRACE_END(open);
    if (is_uv_error(result)) return;
    env->AddUnmanagedFd(result);
    args.GetReturnValue().Set(result);
  }
}
```

可以看到正常的话处理路径依旧是两条, 分别对应这同步以及异步处理, 实现与我们之前分析的 `read` 函数类似, 我们额外关注一下异步实现, 因为 `open` 成功会检查 `fstat` 状态, 之后才是 `read`, 我们直接回到 `read` 函数中去, 查看相关异步调用部分.

### Read 中的异步处理

```c++
// src/node_file.cc:2631
FSReqBase* req_wrap_async = GetReqWrap(args, 5);
CHECK_NOT_NULL(req_wrap_async);
FS_ASYNC_TRACE_BEGIN0(UV_FS_READ, req_wrap_async)
AsyncCall(env, req_wrap_async, args, "read", UTF8, AfterInteger,
          uv_fs_read, fd, &uvbuf, 1, pos);
```

如上, 可以看到先拿到 `FSReqCallback` 实例, 因为 `FSReqBase` 作为其基类, 这里直接 `逆变` 为基类, 如同狗变为动物, 需要注意的是 `FSReqCallback` 类中保存着相关操作信息. 之后调用 `AsyncCall` 函数, 该函数实现如下:

```c++
// src/node_file-inl.h:330
// Returns nullptr if the operation fails from the start.
template <typename Func, typename... Args>
FSReqBase* AsyncCall(Environment* env,
                     FSReqBase* req_wrap,
                     const v8::FunctionCallbackInfo<v8::Value>& args,
                     const char* syscall, enum encoding enc,
                     uv_fs_cb after, Func fn, Args... fn_args) {
  return AsyncDestCall(env, req_wrap, args,
                       syscall, nullptr, 0, enc,
                       after, fn, fn_args...);
}
```

继续追 `AsyncDestCall` 函数, 实现如下:

```c++
// src/node_file-inl.h:307
// Returns nullptr if the operation fails from the start.
template <typename Func, typename... Args>
FSReqBase* AsyncDestCall(Environment* env, FSReqBase* req_wrap,
                         const v8::FunctionCallbackInfo<v8::Value>& args,
                         const char* syscall, const char* dest,
                         size_t len, enum encoding enc, uv_fs_cb after,
                         Func fn, Args... fn_args) {
  CHECK_NOT_NULL(req_wrap);
  req_wrap->Init(syscall, dest, len, enc);
  int err = req_wrap->Dispatch(fn, fn_args..., after);
  if (err < 0) {
    uv_fs_t* uv_req = req_wrap->req();
    uv_req->result = err;
    uv_req->path = nullptr;
    after(uv_req);  // after may delete req_wrap if there is an error
    req_wrap = nullptr;
  } else {
    req_wrap->SetReturnValue(args);
  }

  return req_wrap;
}
```

可以看到主要操作函数有, `req_wrap->Init` 和 `req_wrap->Dispatch`, 根据类型我们可以知道来自于 `FSReqBase` 类. 可以猜测 `req_wrap->Init` 应该是保存了操作的相关信息, 实际上确实如此:

```c++
// src/node_file-inl.h:51
void FSReqBase::Init(const char* syscall,
                     const char* data,
                     size_t len,
                     enum encoding encoding) {
  syscall_ = syscall;
  encoding_ = encoding;

  if (data != nullptr) {
    CHECK(!has_data_);
    buffer_.AllocateSufficientStorage(len + 1);
    buffer_.SetLengthAndZeroTerminate(len);
    memcpy(*buffer_, data, len);
    has_data_ = true;
  }
}

FSReqBase::FSReqBuffer&
FSReqBase::Init(const char* syscall, size_t len, enum encoding encoding) {
  syscall_ = syscall;
  encoding_ = encoding;

  buffer_.AllocateSufficientStorage(len + 1);
  has_data_ = false;  // so that the data does not show up in error messages
  return buffer_;
}
```

再看一下 `req_wrap->Dispatch` 函数:

```c++
// src/req_wrap-inl.h:142
template <typename T>
template <typename LibuvFunction, typename... Args>
int ReqWrap<T>::Dispatch(LibuvFunction fn, Args... args) {
  Dispatched();
  // This expands as:
  //
  // int err = fn(env()->event_loop(), req(), arg1, arg2, Wrapper, arg3, ...)
  //              ^                                       ^        ^
  //              |                                       |        |
  //              \-- Omitted if `fn` has no              |        |
  //                  first `uv_loop_t*` argument         |        |
  //                                                      |        |
  //        A function callback whose first argument      |        |
  //        matches the libuv request type is replaced ---/        |
  //        by the `Wrapper` method defined above                  |
  //                                                               |
  //               Other (non-function) arguments are passed  -----/
  //               through verbatim
  int err = CallLibuvFunction<T, LibuvFunction>::Call(
      fn,
      env()->event_loop(),
      req(),
      MakeLibuvRequestCallback<T, Args>::For(this, args)...);
  if (err >= 0) {
    ClearWeak();
    env()->IncreaseWaitingRequestCounter();
  }
  return err;
}
```

可以看到最终是调用了 `CallLibuvFunction<T, LibuvFunction>::Call` 函数, 该函数实现如下, 由语义猜测包装了 `libuv` 函数调用, 最终调用 `libuv` 相关函数:

```c++
// src/req_wrap-inl.h:75
// Detect `int uv_foo(uv_loop_t* loop, uv_req_t* request, ...);`.
template <typename ReqT, typename... Args>
struct CallLibuvFunction<ReqT, int(*)(uv_loop_t*, ReqT*, Args...)> {
  using T = int(*)(uv_loop_t*, ReqT*, Args...);
  template <typename... PassedArgs>
  static int Call(T fn, uv_loop_t* loop, ReqT* req, PassedArgs... args) {
    return fn(loop, req, args...);
  }
};
```

最终调用为 `uv_fs_read(loop, req, fd, &uvbuf, 1, pos, AfterInteger)`, 由于 `uv_fs_read` 有回调函数传入的话就为异步调用, 即走事件循环. 到这里基本上到底了, 除去系统底层及 `libuv` 相关实现, 但是我们还有个问题, 那就是回调 `AfterInteger` 有什么作用呢? 实现代码如下:

```c++
// src/node_file.cc:802
void AfterInteger(uv_fs_t* req) {
  FSReqBase* req_wrap = FSReqBase::from_req(req);
  FSReqAfterScope after(req_wrap, req);
  FS_ASYNC_TRACE_END1(
      req->fs_type, req_wrap, "result", static_cast<int>(req->result))
  int result = static_cast<int>(req->result);
  if (result >= 0 && req_wrap->is_plain_open())
    req_wrap->env()->AddUnmanagedFd(result);

  if (after.Proceed())
    req_wrap->Resolve(Integer::New(req_wrap->env()->isolate(), result));
}
```

这里我们可以关注到 `req_wrap->Resolve` 函数, `resolve` 为 `promise` 中的核心概念, 我们猜测该函数应该与 `promise` 有关, 具体有哪些关系呢? 后续再行文讨论.

## 总结

我们使用一路向下的方式对 `readFileSync` 和 `readFile` 进行了相关内部分析, 其最终调用均为 `libuv` 的 `uv_fs_read` 函数, 由有没有 `cb` 决定是否直接系统调用即同步调用, 或者异步调用, 但是我们不能忽略掉每一层的相关抽象, 这些抽象是有很多作用的, 这对于 `node` 平台的构建有较多考虑. 对于每层给出的抽象后行文再进行讨论, 同时我们遗留下了几个问题, 分别是内置模块加载及注册, 异步如何构建. 后续再进行行文进行讨论. 希望对大家理解 `node` 平台有所帮助.
