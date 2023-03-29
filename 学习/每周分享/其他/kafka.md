# kafka 相关介绍

## kafka 架构

- Producer
生产者即数据的发布者，该角色将消息发布到 Kafka 的 topic 中。broker 接收到生产者发送的消息后，broker 将该消息追加到当前用于追加数据的 segment 文件中。
生产者发送的消息，存储到一个partition中，生产者也可以指定数据存储的 partition。
- Consumer
消费者可以从 broker 中读取数据。消费者可以消费多个 topic 中的数据。
- Topic
在 Kafka 中，使用一个类别属性来划分数据的所属类，划分数据的这个类称为 topic。如果把 Kafka 看做为一个数据库，topic 可以理解为数据库中的一张表，topic 的名字即为表名。
- Partition
topic 中的数据分割为一个或多个 partition。每个 topic 至少有一个 partition。每个 partition 中的数据使用多个 segment 文件存储。partition 中的数据是有序的，partition 间的数据丢失了数据的顺序。如果 topic 有多个 partition，消费数据时就不能保证数据的顺序。在需要严格保证消息的消费顺序的场景下，需要将partition数目设为1。
- Partition oﬀset
每条消息都有一个当前 Partition 下唯一的 64 字节的 oﬀset，它指明了这条消息的位置。
- Replicas of partition
副本是一个分区的备份。副本不会被消费者消费，副本只用于防止数据丢失，即消费者不从为 follower 的 partition 中消费数据，而是从为 leader 的 partition 中读取数据。副本之间是一主多从的关系。
- Broker
Kafka 集群包含一个或多个服务器，服务器节点称为 broker。broker 存储 topic 的数据。如果某 topic 有 N 个 partition，集群有 N 个 broker，那么每个 broker 存储该 topic 的一个 partition。如果某 topic 有 N 个 partition，集群有(N+M)个 broker，那么其中有 N 个 broker 存储该 topic 的一个 partition，剩下的 M 个 broker 不存储该 topic 的partition 数据。如果某 topic 有 N 个 partition，集群中 broker 数目少于 N 个，那么一个 broker 存储该 topic 的一个或多个 partition。在实际生产环境中，尽量避免这种情况的发生，这种情况容易导致 Kafka 集群数据不均衡。
- Leader
每个 partition 有多个副本，其中有且仅有一个作为 Leader，Leader 是当前负责数据的读写的 partition。
- Follower
Follower 跟随 Leader，所有写请求都通过 Leader 路由，数据变更会广播给所有 Follower，Follower 与 Leader 保持数据同步。如果 Leader 失效，则从 Follower 中选举出一个新的Leader。
当 Follower 与 Leader 挂掉、卡住或者同步太慢，leader 会把这个 follower 从“in sync replicas”（ISR）列表中删除，重新创建一个 Follower。
- Zookeeper
Zookeeper 负责维护和协调 broker。当 Kafka 系统中新增了 broker 或者某个 broker 发生故障失效时，由 ZooKeeper 通知生产者和消费者。生产者和消费者依据 Zookeeper 的 broker 状态信息与 broker 协调数据的发布和订阅任务。
- AR(Assigned Replicas)
分区中所有的副本统称为AR。
- ISR(In-Sync Replicas)
所有与Leader部分保持一致的副（包括Leader副本在内）本组成ISR。
- OSR(Out-of-Sync-Replicas)
与Leader副本同步滞后过多的副本。

## 环境搭建以及测试

```bash
docker compsoe up -f kafka.yml -d
```

```yml
version: "3"
services:
  zookeeper:
    image: 'bitnami/zookeeper:latest'
    container_name: zookeeper
    restart: unless-stopped
    privileged: true
    ports:
      - '2181:2181'
    volumes:
      - zookeeper_data:/bitnami
      - ./volumes/kafka/zookeeper/etc:/etc
    environment:
      - ALLOW_ANONYMOUS_LOGIN=yes
    networks:
      - kafkanet
  kafka:
    image: 'bitnami/kafka:latest'
    container_name: kafka
    restart: unless-stopped
    ports:
      - '9092:9092'
    # command: 
    #  /bin/bash -c "mkdir -p /bitnami/kafka/data && chmod -R 777 /bitnami/kafka/data" 
    environment:
      TZ: CST-8
      KAFKA_BROKER_ID: 1
      KAFKA_LISTENERS: "INTERNAL://:29092,EXTERNAL://:9092"
      KAFKA_ADVERTISED_LISTENERS: "INTERNAL://kafka:29092,EXTERNAL://192.168.10.121:9092"
      KAFKA_LISTENER_SECURITY_PROTOCOL_MAP: "INTERNAL:PLAINTEXT,EXTERNAL:PLAINTEXT"
      KAFKA_INTER_BROKER_LISTENER_NAME: "INTERNAL"
      KAFKA_CFG_ZOOKEEPER_CONNECT: zookeeper:2181
      # KAFKA_CFG_LOG_FLUSH_INTERVAL_MESSAGES: 10
      # KAFKA_CFG_LOG_FLUSH_INTERVAL_MS: 1000
      KAFKA_OFFSETS_TOPIC_REPLICATION_FACTOR: 1
      ALLOW_PLAINTEXT_LISTENER: yes
    volumes:
      - kafka_data:/bitnami
    networks:
      - kafkanet
    depends_on:
      - zookeeper

  kafdrop:
    image: obsidiandynamics/kafdrop
    restart: "no"
    container_name: kafdrop
    ports:
       - "9000:9000"
    environment:
       KAFKA_BROKERCONNECT: kafka:29092
       JVM_OPTS: "-Xms16M -Xmx48M -Xss180K -XX:-TieredCompilation -XX:+UseStringDeduplication -noverify"
      #  SCHEMAREGISTRY_AUTH: user1:user1
    networks:
      - kafkanet
    depends_on:
      - zookeeper
      - kafka
    # cpus: '1'
    # mem_limit: 1024m
volumes:
  zookeeper_data:
    driver: local
  kafka_data:
    driver: local
networks:
    kafkanet:
      driver: bridge
```

## 消息可靠性传输

### 消费端消息丢失

- 场景描述

位移提交：对于Kafka中的分区而言，它的每条消息都有唯一的oﬀset，用来表示消息在分区中的位置。对于消费者而言，它也有一个oﬀset的概念，消费者使用oﬀset来表示消费到分区中某个消息所在的位置。单词"oﬀset"可以编译为"偏移量"，也可以翻译为"位移"，在很多的中文资料中都会交叉使用"偏移量"和"位移"这两个词，对于消息在分区中的位置，我们将oﬀset称之为"偏移量"；对于消费者消费到的位置，将oﬀset称为"位移"，有时候也会更明确地称之为"消费位移"（"偏移量"是在讲分区存储层面的内容，"位移"是在讲消费层面的内容）当然对于一条消息而言，它的偏移量和消费者消费它时的消费位移是相等的。
当每一次调用poll()方法时，它返回的是还没有消费过的消息集（当然这个前提是消息以及存储在Kafka中了，并且暂不考虑异常情况的发生），要做到这一点，就需要记录上一次消费时的消费位移。并且这个消费位移必须做持久化保存，而不是单单保存在内存中，否则消费者重启之后就无法知晓之前的消费位移。消费位移存储在Kafka内部的主题__consumer_oﬀsets中。这里把将消费位移存储起来的动作称之为"提交"，消费者在消费完消息之后需要执行消费位移的提交。
默认情况下Kafka的消费位移提交是自动提交，这个由消费者客户端参数enable.auto.commit配置，默认值为true。当然这个默认的自动提交并不是每消费一条消息就提交一次，而是定期提交，这个定期的周期时间由客户端参数auto.commit.interval.ms配置，默认值为5秒。自动位移提交的动作是在poll()方法的逻辑里面完成的，在每次真正向服务端发起拉取请求之前会检查是否可以进行位移提交，如果可以，那么就会提交上一次轮询的位移。
自动位移提交带来为问题：
1、重复消费
2、消息丢失

- 解决方案

解决方案：将自动位移提交更改为手动位移提交

### 生产端消息丢失

生产者将消息发送到Broker中以后，消息还没有被及时消费，此时Broker宕机了，这样就会导致消息丢失。

#### 场景描述
Kafka消息的发送流程：
在消息发送的过程中，涉及到了两个线程——main线程和Sender线程，以及一个线程共享变量——RecordAccumulator。main线程将消息发送给RecordAccumulator，Sender线程会根据指定的条件，不断从RecordAccumulator中拉取消息发送到Kafka broker。

Sender线程拉取消息的条件：
1、缓冲区大小达到一定的阈值（默认是16384byte），可以通过spring.kafka.producer.batch-size进行设定
2、缓冲区等待的时间达到设置的阈值（默认是0）， 可以通过linger.ms属性进行设定

发送消息的三种方式：1、发后即忘 2、同步消息发送 3、异步消息发送

- 只管往kafka发送消息（消息只发送到缓冲区）而并不关心消息是否正确到达。正常情况没什么问题，不过有
些时候（比如不可重试异常）会造成消息的丢失。
- 同步消息发送
- 异步发送 -> 性能较好，可靠性也有保障

#### 解决方案
针对上述情况所产生的消息丢失，可以的解决方案有如下几种：

1、给topic设置replication.factor参数大于1,要求每个partition必须最少有两个副本
2、搭建Kafka集群，让各个分区副本均衡的分配到不同的broker上
3、在producer端设置
acks=all,要求每条数据写入replicas后,才认为写入成功 
ack=0， 生产者在成功写入消息之前不会等待任何来自服务器的响应。如果出现问题生产者是感知不到的，消息就丢失了。不过因为生产者不需要等待服务器响应，所以它可以以网络能够支持的最大速度发送消息，从而达到很高的吞吐量。
ack=1，默认值为1，只要集群的首领节点收到消息，生产者就会收到一个来自服务器的成功响应。如果消息无法达到首领节点（比如首领节点崩溃，新的首领还没有被选举出来），生产者会收到一个错误响应，为了避免数据丢失，生产者会重发消息（Kafka生产者内部机制）。但是，这样还有可能会导致数据丢失，如果收到写成功通知，此时首领节点还没来的及同步数据到follower节点，首领节点崩溃，就会导致数据丢失。
ack=-1/all， 只有当所有参与复制的节点都收到消息时，生产者会收到一个来自服务器的成功响应，这种模式是最安全的，它可以保证不止一个服务器收到消息。
当生产者发送消息完毕以后，没有收到Broker返回的ack，此时就会触发重试机制或者抛出异常。我们可以通过retries参数设置重试次数（spring boot和Kafka整合默认的重试次数为0），发送客户端会进行重试直到broker返回ack；当消息重试了指定次数以后，还没有收到服务端的ack，此时就会抛出异常。 如果我们还需要保证消息的顺序性，那么我们就需要将 max.in.ﬂight.requests.per.connection设置为1，该参数指定了生产者在收到服务器响应之前可以发送多少个消息。它的值越高，就会占用越多的内存，不过也会提升吞吐量。把它设为 1 可以保证消息是按照发送的顺序写入服务器的，即使发生了重试。

## 消息重复

### 生产

借助事务

### 消费

数据消费完没有及时提交oﬀset到broker。

解决方案: 
1、取消自动自动提交
每次消费完或者程序退出时手动提交。
2、下游做幂等
一般的解决方案是让下游做幂等。

## 高可用

## kafka 为什么这么快

## 代码示例

```ts
// 手动 offset
import { Kafka, logLevel } from 'kafkajs';

const client = new Kafka({
    clientId: 'transactional-client',
    brokers: ['192.168.10.121:9092'],
    logLevel: logLevel.DEBUG,
});
const producer = client.producer({
    transactionalId: 'my-transactional-producer',
    maxInFlightRequests: 1,
    idempotent: true,
});

const consumer = client.consumer({
    groupId: 'my-group',
    heartbeatInterval: 1000,
});

let num = 0;
(async () => {
    await consumer.connect();
    await consumer.subscribe({
        topics: ['auditlogs.wathers'],
        fromBeginning: true,
    });
    await consumer.run({
        autoCommit: false,
        eachMessage: async ({
            topic,
            partition,
            message,
            heartbeat,
            pause,
        }) => {
            console.log('offset: ', message.offset);
            console.log('num: ', num++);
            console.log({
                key: message.key?.toString(),
            });
            consumer.commitOffsets([
                { topic, partition, offset: (+message.offset + 1).toString() },
            ]);
        },
    });
})();
```

