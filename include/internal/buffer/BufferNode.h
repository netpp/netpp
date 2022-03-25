//
// Created by gaojian on 2022/2/23.
//

#ifndef NETPP_BUFFERNODE_H
#define NETPP_BUFFERNODE_H

#include <cstdint>

namespace netpp::internal::buffer {
/**
 * @brief A single buffer node item
 */
struct BufferNode {
	/**
	 * @brief The type of buffer length
	 */
	using LengthType = std::uint64_t;
	/**
	 * @brief The default node size
	 */
	constexpr static LengthType BufferNodeSize = 1024;

	BufferNode();
	BufferNode(const BufferNode &other);

	// constexpr static int maxTimeToLive = 10;
	/**
	 * @brief The offset where buffer start to read
	 */
	LengthType start;
	/**
	 * @brief The offset where buffer start to write
	 */
	LengthType end;

	// int timeToLive;
	char buffer[BufferNodeSize];    // buffer
};
}

#endif //NETPP_BUFFERNODE_H
