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
cmake >= 3.18.0  
clang >= 10.0.1
### Build with cmake
```
git clone https://github.com/netpp/netpp.git
mkdir build
cd build
cmake ../
make netpp -j8
```
## How to use netpp
Examples are under /example directory.

### Start
To define business logic, you should subclass interface Events. 
```c++
#include "Events.h"
class Echo : public netpp::Events {
public:
    void onMessageReceived(netpp::Channel *channel) override;
    std::unique_ptr<netpp::Events> clone() override;
};
```
Events class provides methods:
* onConnected
* onMessageReceived
* onWriteCompleted
* onDisconnect
* onError

In main() function, you should first initialize logger and create an event loop dispatcher.
```c++
netpp::initLogger();
netpp::core::EventLoopDispatcher dispatcher;
```
Then you can start a server using
```c++
netpp::TcpServer server(&dispatcher, std::make_unique<Echo>());
server.listen((netpp::Address("0.0.0.0", 12345)));
```
or a client using.
```c++
netpp::TcpClient client(std::make_unique<Echo>());
client.connect(netpp::Address("127.0.0.1", 12345));
```
The TcpServer's listen or TcpClient's connect won't start an event loop, call this at end.
```c++
dispatcher.startLoop();
```
For more information, take a look at /example/echo.

## The classes you might needs
* TcpClient - the client
* TcpServer - the server
* Events - the interface provide event handlers
* Timer - the timer
* Address - wrap of linux sockaddr_in
* Channel - a socket read/write channel
* ByteArray - a big ending byte buffer
