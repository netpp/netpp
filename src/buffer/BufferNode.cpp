//
// Created by gaojian on 2022/2/23.
//

#include "buffer/BufferNode.h"
#include <cstring>

namespace netpp {
BufferNode::BufferNode()
	: start{0}, end{0}, buffer{}
{}

BufferNode::BufferNode(const BufferNode &other)
		: start{other.start}, end{other.end}, buffer{}
{
	std::memcpy(buffer, other.buffer, BufferNodeSize);
}
}
