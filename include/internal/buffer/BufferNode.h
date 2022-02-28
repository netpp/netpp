//
// Created by gaojian on 2022/2/23.
//

#ifndef NETPP_BUFFERNODE_H
#define NETPP_BUFFERNODE_H

#include <cstdint>

namespace netpp::internal::buffer {
struct BufferNode {
	using LengthType = std::uint64_t;
	constexpr static LengthType BufferNodeSize = 1024;

	BufferNode();
	BufferNode(const BufferNode &other);

	// constexpr static int maxTimeToLive = 10;
	LengthType start;    // the offset of buffer read
	LengthType end;    // the offset of buffer write
	// int timeToLive;
	char buffer[BufferNodeSize];    // buffer
};
}

#endif //NETPP_BUFFERNODE_H
