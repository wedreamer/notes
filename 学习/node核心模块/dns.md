# 一个 dns 函数的执行过程

```js
const { Resolver } = require('node:dns');
const resolver = new Resolver();
resolver.setServers(['4.4.4.4']);

// This request will use the server at 4.4.4.4, independent of global settings.
resolver.resolve4('example.org', (err, addresses) => {
  // ...
});
```

## js

就拿[dns官方文档](https://nodejs.org/dist/latest-v18.x/docs/api/dns.html#class-dnsresolver)上的实例函数进行说明.

```js
// dns 标准模块
const { Resolver } = require('node:dns');
const resolver = new Resolver();
// 设置 dns server
resolver.setServers(['4.4.4.4']);

// This request will use the server at 4.4.4.4, independent of global settings.
// 进行 dns 解析, 注册回调函数处理
resolver.resolve4('example.org', (err, addresses) => {
  // ...
});
```

dns 属于内置模块, 回调源码位置在 `lib/dns.js`, promise 位置在 `lib/internal/dns/promises.js`, 示例代码使用回调注册处理。

Resolver 的导出
```js
// lib/dns.js
module.exports = {
  // ....

  Resolver,
  setDefaultResultOrder,
  setServers: defaultResolverSetServers,

  // ...
};
```

Resolver 相关定义
```js
// lib/internal/dns/utils.js
const {
  // 核心类, node 对 cares_wrap 的包装
  ChannelWrap,
} = internalBinding('cares_wrap');
class Resolver {
  constructor(options = undefined) {
    const timeout = validateTimeout(options);
    const tries = validateTries(options);
    // 初始化
    // 超时时间, 以及重试次数
    this._handle = new ChannelWrap(timeout, tries);
  }

  // 设置 dns 服务器
  setServers(servers) {
    validateArray(servers, 'servers');

    // Cache the original servers because in the event of an error while
    // setting the servers, c-ares won't have any servers available for
    // resolution.
    const orig = this._handle.getServers() || [];
    const newSet = [];

    // 遍历以及合法检查
    ArrayPrototypeForEach(servers, (serv, index) => {
      validateString(serv, `servers[${index}]`);
      let ipVersion = isIP(serv);

      if (ipVersion !== 0)
        return ArrayPrototypePush(newSet, [ipVersion, serv, IANA_DNS_PORT]);

      const match = RegExpPrototypeExec(IPv6RE, serv);

      // Check for an IPv6 in brackets.
      if (match) {
        ipVersion = isIP(match[1]);

        if (ipVersion !== 0) {
          const port = NumberParseInt(
            RegExpPrototypeSymbolReplace(addrSplitRE, serv, '$2')) || IANA_DNS_PORT;
          return ArrayPrototypePush(newSet, [ipVersion, match[1], port]);
        }
      }

      // addr::port
      const addrSplitMatch = RegExpPrototypeExec(addrSplitRE, serv);

      if (addrSplitMatch) {
        const hostIP = addrSplitMatch[1];
        const port = addrSplitMatch[2] || IANA_DNS_PORT;

        ipVersion = isIP(hostIP);

        if (ipVersion !== 0) {
          return ArrayPrototypePush(
            newSet, [ipVersion, hostIP, NumberParseInt(port)]);
        }
      }

      throw new ERR_INVALID_IP_ADDRESS(serv);
    });

    // 核心操作
    // 实际调用的是 ChannelWrap 对象函数 -> src/cares_wrap.cc
    const errorNumber = this._handle.setServers(newSet);

    if (errorNumber !== 0) {
      // Reset the servers to the old servers, because ares probably unset them.
      this._handle.setServers(ArrayPrototypeJoin(orig, ','));
      const err = strerror(errorNumber);
      throw new ERR_DNS_SET_SERVERS_FAILED(err, servers);
    }
  }
}
```

设置 dns 服务器

```c++
// src/cares_wrap.cc
// js 函数名 'setServers' -> 搜索 可以定位到 src/cares_wrap.cc
// 原因在于核心模块在 node 初始化的时候都会执行该文件最下面定义的初始化 宏, 为了使 c++ 定义的相关元素和功能通过 v8 暴露给待运行的 js
Local<FunctionTemplate> channel_wrap =
      NewFunctionTemplate(isolate, ChannelWrap::New);
  channel_wrap->InstanceTemplate()->SetInternalFieldCount(
      ChannelWrap::kInternalFieldCount);
  channel_wrap->Inherit(AsyncWrap::GetConstructorTemplate(env));
 
SetProtoMethod(isolate, channel_wrap, "queryAny", Query<QueryAnyWrap>);
SetProtoMethod(isolate, channel_wrap, "queryA", Query<QueryAWrap>); // 示例代码中, 之后需要执行的函数
// ...
SetProtoMethod(isolate, channel_wrap, "setServers", SetServers); // +++++
SetProtoMethod(isolate, channel_wrap, "setLocalAddress", SetLocalAddress);
SetProtoMethod(isolate, channel_wrap, "cancel", Cancel);

SetConstructorFunction(context, target, "ChannelWrap", channel_wrap); // ++++ 函数模板添加到 v8 上下文中
```

设置 dns 服务器 c++
```c++
// src/cares_wrap.cc
void SetServers(const FunctionCallbackInfo<Value>& args) {
  // 根据参数获取当前的 env
  Environment* env = Environment::GetCurrent(args);
  // 声明包装好的信道
  ChannelWrap* channel;
  ASSIGN_OR_RETURN_UNWRAP(&channel, args.Holder());

  // 当前信道已经有活跃的请求
  if (channel->active_query_count()) {
    return args.GetReturnValue().Set(DNS_ESETSRVPENDING);
  }

  // 保证参数第一个一定时数组
  CHECK(args[0]->IsArray());

  // 转化成数组
  Local<Array> arr = args[0].As<Array>();

  // 数组长度
  uint32_t len = arr->Length();

  if (len == 0) {
    int rv = ares_set_servers(channel->cares_channel(), nullptr);
    return args.GetReturnValue().Set(rv);
  }

  // 声明等长的 向量 保存地址和端口信息
  std::vector<ares_addr_port_node> servers(len);
  ares_addr_port_node* last = nullptr;

  int err;

  for (uint32_t i = 0; i < len; i++) {
    // 乱七八糟的相关检查
    CHECK(arr->Get(env->context(), i).ToLocalChecked()->IsArray());

    Local<Array> elm = arr->Get(env->context(), i).ToLocalChecked().As<Array>();

    CHECK(elm->Get(env->context(),
                   0).ToLocalChecked()->Int32Value(env->context()).FromJust());
    CHECK(elm->Get(env->context(), 1).ToLocalChecked()->IsString());
    CHECK(elm->Get(env->context(),
                   2).ToLocalChecked()->Int32Value(env->context()).FromJust());

    int fam = elm->Get(env->context(), 0)
        .ToLocalChecked()->Int32Value(env->context()).FromJust();
    node::Utf8Value ip(env->isolate(),
                       elm->Get(env->context(), 1).ToLocalChecked());
    int port = elm->Get(env->context(), 2)
        .ToLocalChecked()->Int32Value(env->context()).FromJust();

    ares_addr_port_node* cur = &servers[i];

    cur->tcp_port = cur->udp_port = port;
    // 根据 ip v4, v6 检查连通性
    switch (fam) {
      case 4:
        cur->family = AF_INET;
        // libuv 工具函数, 同步检查 ip 是否能通 -> v4
        err = uv_inet_pton(AF_INET, *ip, &cur->addr);
        break;
      case 6:
        cur->family = AF_INET6;
        // libuv 工具函数, 同步检查 ip 是否能通 -> v6
        err = uv_inet_pton(AF_INET6, *ip, &cur->addr);
        break;
      default:
        CHECK(0 && "Bad address family.");
    }

    // 只要有一个失败就退出循环
    if (err)
      break;

    cur->next = nullptr;

    if (last != nullptr)
      last->next = cur;

    last = cur;
  }

  if (err == 0)
    err = ares_set_servers_ports(channel->cares_channel(), &servers[0]);
  else
    err = ARES_EBADSTR;

  if (err == ARES_SUCCESS)
    channel->set_is_servers_default(false);
  // 返回错误值
  args.GetReturnValue().Set(err);
}
```

resolve4 

```js
// lib/dns.js
Resolver.prototype.resolve4 = resolveMap.A = resolver('queryA');
```

```js
// lib/dns.js
function resolver(bindingName) {
  function query(name, /* options, */ callback) {
    let options;
    if (arguments.length > 2) {
      options = callback;
      callback = arguments[2];
    }

    validateString(name, 'name');
    validateFunction(callback, 'callback');

    const req = new QueryReqWrap();
    req.bindingName = bindingName; // queryA
    req.callback = callback; // 我们自己注册的回调函数
    req.hostname = name;
    req.oncomplete = onresolve; // 之后会用到, 当成功之后解析结果的时候, 会回调这个函数, this 指向当前作用域
    req.ttl = !!(options && options.ttl);
    // 最终调用的函数
    // req, toASCII(name)
    // 当前 this 指向 Resolver, _handle 就是 ChannelWrap
    // 等效调用 new ChannelWrap[bindingName](req, toASCII(name))
    const err = this._handle[bindingName](req, toASCII(name));
    if (err) throw dnsException(err, bindingName, name);
    if (hasObserver('dns')) {
      startPerf(req, kPerfHooksDnsLookupResolveContext, {
        type: 'dns',
        name: bindingName,
        detail: {
          host: name,
          ttl: req.ttl,
        },
      });
    }
    return req;
  }
  ObjectDefineProperty(query, 'name', { __proto__: null, value: bindingName });
  return query;
}
```

## c++

现在来看 c++ 中对应的代码

```c++
// src/cares_wrap.cc
SetProtoMethod(isolate, channel_wrap, "queryA", Query<QueryAWrap>);
```

```c++
// src/cares_wrap.cc
// 此时 Wrap 为 QueryAWrap
namespace {
template <class Wrap>
static void Query(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  ChannelWrap* channel;
  ASSIGN_OR_RETURN_UNWRAP(&channel, args.Holder());

  CHECK_EQ(false, args.IsConstructCall());
  CHECK(args[0]->IsObject());
  CHECK(args[1]->IsString());

  // req
  Local<Object> req_wrap_obj = args[0].As<Object>();
  // name
  Local<String> string = args[1].As<String>();
  // init wrap
  auto wrap = std::make_unique<Wrap>(channel, req_wrap_obj);

  node::Utf8Value name(env->isolate(), string);
  // 增加计数
  channel->ModifyActivityQueryCount(1);
  int err = wrap->Send(*name);
  if (err) {
    channel->ModifyActivityQueryCount(-1);
  } else {
    // Release ownership of the pointer allowing the ownership to be transferred
    // 释放指针的所有权，允许所有权转移
    USE(wrap.release());
  }

  args.GetReturnValue().Set(err);
}
```

发送请求

```c++
// src/cares_wrap.h
// 类型别名
using QueryAWrap = QueryWrap<ATraits>;
```

```c++
// src/cares_wrap.cc
int ATraits::Send(QueryWrap<ATraits>* wrap, const char* name) {
  // 发送请求
  wrap->AresQuery(name, ns_c_in, ns_t_a);
  return 0;
}
```

```c++
void AresQuery(const char* name, int dnsclass, int type) {
    channel_->EnsureServers();
    TRACE_EVENT_NESTABLE_ASYNC_BEGIN1(
      TRACING_CATEGORY_NODE2(dns, native), trace_name_, this,
      "name", TRACE_STR_COPY(name));
    ares_query(
        channel_->cares_channel(),
        name,
        dnsclass,
        type,
        Callback,
        MakeCallbackPointer());
}
```

```c++

```

## c++ callback js

## todo
