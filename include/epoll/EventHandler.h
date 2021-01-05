//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTHANDLER_H
#define NETPP_EVENTHANDLER_H

#include "EpollEvent.h"
#include "Events.h"
#include <type_traits>

namespace netpp::epoll {
class EventHandler {
public:
	explicit EventHandler(int fd)
		: m_fd{fd}
	{}
	virtual ~EventHandler() = default;

	virtual void handleRead() = 0;
	virtual void handleWrite() = 0;
	virtual void handleError() = 0;
	virtual void handleDisconnect() = 0;

	inline int fd() const { return m_fd; }

protected:
	int m_fd;
	std::unique_ptr<EpollEvent> m_epollEvent;
};

/*template<typename T>
class ConditionDeleter {
public:
	explicit ConditionDeleter(bool doDelete = true) : _delete{doDelete} {}
	void operator()([[maybe_unused]] T *p) const { if (_delete) delete p; }

private:
	bool _delete;
};

using EventHandlerPtr = std::unique_ptr<EventHandler, ConditionDeleter<EventHandler>>;

inline EventHandlerPtr makeEventHandlerPtrFinder(EventHandler *rawPtr)
{
	return EventHandlerPtr(rawPtr, ConditionDeleter<EventHandler>(false));
}

template<typename SubClass, typename... Args>
inline std::unique_ptr<SubClass, ConditionDeleter<EventHandler>> makeEventHandlerPtr(Args &&... args)
{
	static_assert(std::is_base_of_v<EventHandler, SubClass>, "Must inherited EventHandler");
	return std::unique_ptr<SubClass, ConditionDeleter<EventHandler>>(new SubClass(std::forward<Args>(args)...), ConditionDeleter<EventHandler>(true));
}*/
}

#endif //NETPP_EVENTHANDLER_H
