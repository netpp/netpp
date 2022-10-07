//
// Created by 17271 on 2022/6/5.
//

#ifndef NETPP_IODEVICE_H
#define NETPP_IODEVICE_H

#include <memory>

namespace netpp {
class ByteArray;
class Address;
class BufferGather;
class IODevice {
public:
	virtual ~IODevice() noexcept = default;

	virtual void open() = 0;
	virtual void close() = 0;
	virtual int fileDescriptor() = 0;

	virtual void read(std::shared_ptr<BufferGather> buffer) = 0;
	virtual std::size_t write(std::shared_ptr<BufferGather> buffer) = 0;
};
}

#endif //NETPP_IODEVICE_H
