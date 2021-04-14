//
// Created by gaojian on 2020/7/9.
//

#include "Channel.h"
#include "internal/handlers/TcpConnection.h"
#include "ByteArray.h"
#include "internal/socket/SocketEnums.h"

namespace netpp {
Channel::Channel(std::weak_ptr<internal::handlers::TcpConnection> connection,
				 std::weak_ptr<ByteArray> writeByteArray, std::weak_ptr<ByteArray> readByteArray)
				 : _connection{std::move(connection)}, _writeArray{std::move(writeByteArray)}, _readArray{std::move(readByteArray)}
{}

void Channel::send()
{
	auto connection = _connection.lock();
	if (connection)
		connection->sendInLoop();
}

void Channel::close()
{
	auto connection = _connection.lock();
	if (connection)
		connection->closeAfterWriteCompleted();
}

bool Channel::writeInt8(int8_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeInt8(value);
		return true;
	}
	return false;
}

bool Channel::writeInt16(int16_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeInt16(value);
		return true;
	}
	return false;
}

bool Channel::writeInt32(int32_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeInt32(value);
		return true;
	}
	return false;
}

bool Channel::writeInt64(int64_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeInt64(value);
		return true;
	}
	return false;
}

bool Channel::writeUInt8(uint8_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeUInt8(value);
		return true;
	}
	return false;
}

bool Channel::writeUInt16(uint16_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeUInt16(value);
		return true;
	}
	return false;
}

bool Channel::writeUInt32(uint32_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeUInt32(value);
		return true;
	}
	return false;
}

bool Channel::writeUInt64(uint64_t value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeUInt64(value);
		return true;
	}
	return false;
}

bool Channel::writeFloat(float value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeFloat(value);
		return true;
	}
	return false;
}

bool Channel::writeDouble(double value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeDouble(value);
		return true;
	}
	return false;
}

bool Channel::writeString(const std::string &value)
{
	auto array = writableArray();
	if (array)
	{
		array->writeString(value);
		return true;
	}
	return false;
}

bool Channel::writeRaw(const char *data, std::size_t length)
{
	auto array = _writeArray.lock();
	if (array)
	{
		array->writeRaw(data, length);
		return true;
	}
	return false;
}

std::size_t Channel::availableRead() const
{ auto array = _readArray.lock(); if (array) return array->readableBytes(); else return 0; }

int8_t Channel::retrieveInt8()
{ auto array = _readArray.lock(); if (array) return array->retrieveInt8(); else return 0; }

int16_t Channel::retrieveInt16()
{ auto array = _readArray.lock(); if (array) return array->retrieveInt16(); else return 0; }

int32_t Channel::retrieveInt32()
{ auto array = _readArray.lock(); if (array) return array->retrieveInt32(); else return 0; }

int64_t Channel::retrieveInt64()
{ auto array = _readArray.lock(); if (array) return array->retrieveInt64(); else return 0; }

uint8_t Channel::retrieveUInt8()
{ auto array = _readArray.lock(); if (array) return array->retrieveUInt8(); else return 0; }

uint16_t Channel::retrieveUInt16()
{ auto array = _readArray.lock(); if (array) return array->retrieveUInt16(); else return 0; }

uint32_t Channel::retrieveUInt32()
{ auto array = _readArray.lock(); if (array) return array->retrieveUInt32(); else return 0; }

uint64_t Channel::retrieveUInt64()
{ auto array = _readArray.lock(); if (array) return array->retrieveUInt64(); else return 0; }

float Channel::retrieveFloat()
{ auto array = _readArray.lock(); if (array) return array->retrieveFloat(); else return 0; }

double Channel::retrieveDouble()
{ auto array = _readArray.lock(); if (array) return array->retrieveDouble(); else return 0; }

std::string Channel::retrieveString(std::size_t length)
{ auto array = _readArray.lock(); if (array) return array->retrieveString(length); else return ""; }

std::size_t Channel::retrieveRaw(char *buffer, std::size_t length)
{ auto array = _readArray.lock(); if (array) return array->retrieveRaw(buffer, length); else return 0; }

std::shared_ptr<ByteArray> Channel::writableArray()
{
	auto connection = _connection.lock();
	if (connection && connection->currentState() == internal::socket::TcpState::Established)
	{
		return _writeArray.lock();
	}
	return nullptr;
}

int Channel::channelId()
{
	auto connection = _connection.lock();
	if (connection)
		return connection->connectionId();
	else
		return -1;
}
}
