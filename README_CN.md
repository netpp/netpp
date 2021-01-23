# Netpp - 现代C++网络库
netpp是一个基于事件的现代网络库，基于reactor和epoll。  
中文 [English](https://github.com/netpp/netpp/blob/master/README.md)
## 依赖库
* 日志库 [spdlog](https://github.com/gabime/spdlog)
* 测试框架 [gtest](https://github.com/google/googletest)
## 如何编译
### 依赖
cmake >= 3.18.0  
clang >= 10.0.1
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

应该首先初始化日志和创建事件循环分派器
```c++
netpp::initLogger();
netpp::core::EventLoopDispatcher dispatcher;
```
在此基础上，开启一个服务器，并指派事件处理器
```c++
std::unique_ptr<netpp::Events<Echo>> events = std::make_unique<netpp::Events<Echo>>(Echo());
netpp::TcpServer server(&dispatcher, std::move(events));
server.listen((netpp::Address("0.0.0.0", 12345)));
```
或者客户端
```c++
std::unique_ptr<netpp::Events<Echo>> events = std::make_unique<netpp::Events<Echo>>(Echo());
netpp::TcpClient client(&dispatcher, std::move(events));
client.connect(netpp::Address("127.0.0.1", 12345));
```
进行事件循环
```c++
dispatcher.startLoop();
```
查看/example/*中的示例来获取更详细的信息。

## 可能用到的一些类
* TcpClient - 客户端
* TcpServer - 服务端
* Events - 提供事件处理器的接口
* Timer - 计时器
* Address - linux结构体sockaddr_in的封装
* Channel - 对于一个tcp连接读写通道的描述
* ByteArray - 一个以大端方式保存的字节缓冲区
