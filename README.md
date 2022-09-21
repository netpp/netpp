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
* async dns [c-ares](https://www.github.com/c-ares/c-ares)
* uri parse(include/http/Uri.h src/http/Uri.cpp) based on [cpprestsdk](https://github.com/microsoft/cpprestsdk)
## How to compile
### Requires
* cmake >= 3.18.0
* c++20 supports
### Build with cmake
```shell
git clone https://github.com/netpp/netpp.git
git submodule update --init
mkdir build
cd build
cmake ../
make netpp -j8
```
## Generate document
use doxygen to generate, graphviz is required, documents were exported in ./doc
```shell
sudo pacman -S doxygen graphviz
mkdir build
cd build
cmake ../
make doc
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
