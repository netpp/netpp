# netpp
An event based, modern c++ network library on Linux
=======
[中文](https://github.com/netpp/netpp/blob/master/README_CN.md) English  
netpp is an event based, modern c++ network library, based on reactor pattern and epoll, supporting features:  
* linked buffer node
* none virtual user interface
* one loop per thread
* handle signal
* kick idle connection
## Dependent libs
* Logging library [spdlog](https://github.com/gabime/spdlog)
* Testing [gtest](https://github.com/google/googletest)
## How to compile
### Requires
* cmake >= 3.16.0
* c++17 supports
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
## How to use netpp
Examples are under /example directory.

### Start
Define bevent handler to implement business logic. 
```c++
class Echo {
public:
    void onMessageReceived(std::shared_ptr<netpp::Channel> channel);
};
```
netpp provides following events:
* onConnected
* onMessageReceived
* onWriteCompleted
* onDisconnect
* onError
* onSignal

Create an event loop dispatcher.
```c++
netpp::core::EventLoopDispatcher dispatcher;
```
Create a server, and assign event handler
```c++
netpp::Events<Echo> events(std::make_shared<Echo>());
netpp::TcpServer server(&dispatcher, netpp::Address("0.0.0.0", 12345), std::move(events));
server.listen();
```
or a client using.
```c++
netpp::Events<Echo> events(std::make_shared<Echo>());
netpp::TcpClient client(&dispatcher, netpp::Address("127.0.0.1", 12345), std::move(events));
client.connect();
```
Start an event loop.
```c++
dispatcher.startLoop();
```
For more information, take a look at example/*.
