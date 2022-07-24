//
// Created by 17271 on 2022/6/5.
//

#ifndef NETPP_IODEVICE_H
#define NETPP_IODEVICE_H

#include <memory>

namespace netpp {
class ByteArray;
class Address;
class BufferIOConversion;
class IODevice {
public:
	virtual ~IODevice() noexcept = default;

	virtual void open() = 0;
	virtual void close() = 0;
	virtual int fileDescriptor() = 0;

	virtual void read(std::unique_ptr<BufferIOConversion> &&buffer) = 0;
	virtual bool write(std::unique_ptr<BufferIOConversion> &&buffer) = 0;
};
}
#endif //NETPP_IODEVICE_H
