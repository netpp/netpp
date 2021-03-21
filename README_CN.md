# netpp
中文 [English](https://github.com/netpp/netpp/blob/master/README.md)  
## netpp是什么
netpp是基于事件通知的TCP网络库，通过reactor和epoll实现，仅支持Linux平台，支持特性：
* 链式的缓存节点
* 非虚函数的用户接口
* 每个事件循环属于不同线程
* 信号处理
* 踢掉空连接
## 依赖库
* 日志库 [spdlog](https://github.com/gabime/spdlog)
* 测试框架 [gtest](https://github.com/google/googletest)
## 如何编译
### 依赖
* cmake >= 3.18.0
* 支持C++17的编译器
### 使用cmake
```
git clone https://github.com/netpp/netpp.git
git submodule init
git submodule update
mkdir build
cd build
cmake ../
make netpp -j8
```
## 开始
Netpp提供非虚函数的方法作为接口通知事件，定义事件处理类，实现对应方法并注入netpp::Events。
```c++
class Echo {
public:
    void onMessageReceived(std::shared_ptr<netpp::Channel> channel);
};
netpp::Events events(std::make_shared<Echo>());
```
事件循环分派器用于创建并指派事件循环。
```c++
netpp::core::EventLoopDispatcher dispatcher;
```
在此基础上，开启一个服务器监听，指定分派器和事件处理方法
```c++
netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
server.listen();
```
或者客户端连接
```c++
netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
client.connect();
```
开始事件循环
```c++
dispatcher.startLoop();
```
查看example/*中的示例来获取更详细的信息。

***
感谢jetbrains为该项目提供全家桶的[开源授权](https://jb.gg/OpenSource)
