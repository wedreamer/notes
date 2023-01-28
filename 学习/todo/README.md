- cjs esm 模块之间的不一样
- 乐观锁, 悲观锁
- c++ 内存管理(内存堆)
- c++ 多线程编程
- java 内存模型
- TAS 学习
- 文件系统中部分写的问题
- mysql 中的 区 块 页 段 表空间
- 幻读以及解决方案
- mysql 启动过程
- 一句 sql 语句在 mysql 中的执行过程
- epoll 是同步还是异步
- 阻塞和非阻塞, 同步以及异步之间的相关概念
- 线程间同步, 进程间通信
- 粘包问题

- 设计模式 - c++
- 马士兵金三银四
- go 进阶训练营
- 凤凰架构
- golang 相关书以及面试题
- 技术相关思考
- mysql 书再读
- node 源码
- nginx 源码
- redis 源码
- tcp/ip 详解
- 算法再思考 

- docker k8s 运行原理
- kvm 虚拟化技术
- 分布式存储与扩展
- zookeeper raft
- 协程
- GraphQL
- 分布式文件系统
- 分布式事务的优缺点
- InnoDB ACID 实现
- 分布式锁相关问题
- MVCC 的实现原理
- weakmap weakset
- 磁盘 IO(顺序 IO, 并发 IO)
- 磁盘和固态的差别
- hash 索引与 hash 冲突
- redis 数据类型与底层数据类型
- nginx 功能概览
- hash 桶
- 一致性 hash
- webscoket
- 全链路压测

- v8 gc
- 分布式事务
- docker k8s
- node 对底层线程池的相关抽象


## InnoDB 相关问题
- 为什么需要内存和磁盘两种双链表并叙述他们的使用场景和异同点
- 为什么 InnoDB 使用较多的是不带链的哈希表, 而不是功能更强的带链哈希表
- 请简述 InnoDB 使用的伙伴系统与 Liunx 内核的伙伴系统有何不同
- InnoDB 存储引擎不支持递归的 s-latch, 若要实现该功能需要进行怎样大致的修改
- InnoDB 存储引擎实现的 mutex 和 latch 是对 short-term critical section 进行互斥的数据结构, long-term critical section 使用何种数据结构
- 跳跃表的实现
- mspseak setseak 的实现
- 装饰器原理
- 削峰 限流 算法及其原理
- mysql 主从复制的实现
- dtm 如何解决 tcc 的相关问题
- 散列表中桶的概念
- 虚拟机技术
- 垃圾回收相关技术
- 阻塞, 非阻塞, 同步, 异步
- mysql 对不同的事务隔离级别是如何处理的
> 1. 脏读(赃数据却截然不同, 脏数据是指未提交的数据, 如果读到了脏数据, 即一个事务可以读到另外一个事务中未提交的数据, 则显然违反了数据库的隔离性)(mvcc)
> 2. 不可重复读(不可重复读是指一个事务内部多次读取同一数据集合. 在这个事务还没有结束时, 另外一个事务也访问该同一数据集合, 并做了一些 DML 操作. 因此, 在第一个事务中的两次读数据之间, 由于第二个事务的修改, 那么第一个事务两次读到的数据可能是不一样的. 这样就发生了一个事务内两次读到的数据是不一样的情况, 这种情况称为不可重复读)(复用 mvcc 的 read view)
> 3. 幻读是指同一事务下, 连续执行两次同样的 sql 语句可能导致不同的结果, 第二次的 sql 语句可能会返回之前不存在的行(在RR的隔离级别下，Innodb使用MVCC和next-key locks解决幻读，MVCC解决的是普通读（快照读）的幻读，next-key locks解决的是当前读情况下的幻读)
[面试官：你说熟悉MySQL，那来谈谈InnoDB怎么解决幻读的？](https://zhuanlan.zhihu.com/p/252321589)


## v8 相关优化
- 参数适配器
- 数组排序
- 后台进程编译字节码
- bitint
- 代码缓存
- 自定义启动快照
- dataview
- 21 种元素种类
- 嵌入式内置组件
- 异步优化
- for in
- 快速属性
- 内联高速缓存
- 使用新的类功能更快地初始化实例
- 隐藏 hash-code
- c++ 垃圾回收
- ES 6+ 的支持
- Ignition
- TurboFan
- Sparkplug
- Indicium
- 代码缓存
- 国际化 intl
- 无 JIT V8
- 懒惰反序列化
- 去优化函数的懒惰解链接
- Math.random
- 非回溯正则表达式引擎
- proxy 优化
- Orinoco V8主要是并发和并行垃圾收集器
- 指针压缩
- 惰性解析
- 扫描优化器
- 取消嵌入内置组件
- 松弛跟踪
- 加速元素传播

## v8 安全
- 禁用逃逸分析
- 哈希泛洪
- c++ 内存安全

## v8 垃圾回收
- 并发标记
- 并发收集处理
- 黑色分配
- 分代堆管理(代际垃圾回收器)
- 复制回收
- 油锅是一个基于跟踪的垃圾回收器

## ZGC 垃圾回收



## 面试计划安排
- mysql InnoDB
- 分布式事务
- nodejs libuv v8
- redis
- mysql 优化
- redis
- 网络
- nginx
- rabbitmq
- docker k8s
- vue3
- html css
- 浏览器原理

## 平常的一些问题




