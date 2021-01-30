# netpp
An event based, modern c++ network library
=======
netpp is an event based, modern c++ network library, based on reactor pattern and epoll.  
[中文](https://github.com/netpp/netpp/blob/master/README_CN.md) English
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

Firstly initialize logger and create an event loop dispatcher.
```c++
netpp::initLogger();
netpp::core::EventLoopDispatcher dispatcher;
```
Create a server, and assign event handler
```c++
std::unique_ptr<netpp::Events<Echo>> events = std::make_unique<netpp::Events<Echo>>(Echo());
netpp::TcpServer server(&dispatcher, std::move(events));
server.listen((netpp::Address("0.0.0.0", 12345)));
```
or a client using.
```c++
std::unique_ptr<netpp::Events<Echo>> events = std::make_unique<netpp::Events<Echo>>(Echo());
netpp::TcpClient client(&dispatcher, std::move(events));
client.connect(netpp::Address("127.0.0.1", 12345));
```
Start an event loop.
```c++
dispatcher.startLoop();
```
For more information, take a look at /example/*.

## The classes you might needs
* TcpClient - the client
* TcpServer - the server
* Events - the interface provide event handlers
* Timer - the timer
* Address - wrap of linux sockaddr_in
* Channel - a socket read/write channel
* ByteArray - a big ending byte buffer
