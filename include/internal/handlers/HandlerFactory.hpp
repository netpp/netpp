//
// Created by gaojian on 22-5-2.
//

#ifndef NETPP_HANDLERFACTORY_HPP
#define NETPP_HANDLERFACTORY_HPP

#include <memory>

namespace netpp::internal::handlers {
class HandlerFactory {
	template<typename Handler, typename ... Args>
	static std::shared_ptr<Handler> createHandler(Args &&... args)
	{
		auto handler = std::make_shared<Handler>(std::forward(args));
		make_unique<epoll::EpollEvent>(loop->getPoll(), handler, connector->m_socket->fd())
	}
};
}
#endif //NETPP_HANDLERFACTORY_HPP
