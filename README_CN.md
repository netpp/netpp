# Netpp - 现代C++网络库
中文 [English](https://github.com/netpp/netpp/blob/master/README.md)  
netpp是Linux平台上基于reactor和epoll的现代网络库，提供特性：  
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
## 如何使用netpp
示例代码位于/example目录下。

### 开始
需要定义事件处理器实现业务逻辑。
```c++
class Echo {
public:
    void onMessageReceived(std::shared_ptr<netpp::Channel> channel);
};
```
netpp提供了以下事件：
* onConnected
* onMessageReceived
* onWriteCompleted
* onDisconnect
* onError
* onSignal

创建事件循环分派器
```c++
netpp::core::EventLoopDispatcher dispatcher;
```
在此基础上，开启一个服务器，并指派事件处理器
```c++
netpp::Events<Echo> events(std::make_shared<Echo>());
netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
server.listen();
```
或者客户端
```c++
netpp::Events<Echo> events(std::make_shared<Echo>());
netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
client.connect();
```
进行事件循环
```c++
dispatcher.startLoop();
```
查看example/*中的示例来获取更详细的信息。
