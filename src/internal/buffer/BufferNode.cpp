//
// Created by gaojian on 2022/2/23.
//

#include "internal/buffer/BufferNode.h"
#include <cstring>

namespace netpp::internal::buffer {
BufferNode::BufferNode()
	: start{0}, end{0}, buffer{}
{}

BufferNode::BufferNode(const BufferNode &other)
		: start{0}, end{0}, buffer{}
{
	std::memcpy(buffer, other.buffer, BufferNodeSize);
}
}
