# netpp
[中文](https://github.com/netpp/netpp/blob/master/README_CN.md) English  
## What is netpp
netpp is an event based, modern c++ TCP network library, based on reactor pattern and epoll, on Linux only, supporting features:  
* linked, cow buffer node
* none virtual user interface
* one loop per thread
* handle signal
* kick idle connection
## Dependent libs
* Logging library [spdlog](https://github.com/gabime/spdlog)
* Testing [gtest](https://github.com/google/googletest)
* http parse [llhttp](https://github.com/nodejs/llhttp)
* uri parse(include/http/Uri.h src/http/Uri.cpp) based on [cpprestsdk](https://github.com/microsoft/cpprestsdk)
## How to compile
### Requires
* cmake >= 3.18.0
* c++20 supports
### Build with cmake
```
git clone https://github.com/netpp/netpp.git
git submodule init
git submodule update
mkdir build
cd build
cmake ../
make netpp -j8
```
## Quick start
Netpp provides non-virtual methods as events notify interface, define event handler and inject to netpp::Events. 
```c++
class Echo {
public:
    void onMessageReceived(std::shared_ptr<netpp::Channel> channel);
};
netpp::Events events(std::make_shared<Echo>());
```
An event loop dispatcher for create and dispatch event loop.
```c++
netpp::core::EventLoopDispatcher dispatcher;
```
Create a server, and assign event handler and dispatcher
```c++
netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
server.listen();
```
or a client
```c++
netpp::Events<Echo> events(std::make_shared<Echo>());
netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
client.connect();
```
Start event loop.
```c++
dispatcher.startLoop();
```
For more information, take a look at example/*.
