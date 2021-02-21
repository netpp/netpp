//
// Created by gaojian on 2020/7/5.
//

#ifndef NETPP_EVENTHANDLER_H
#define NETPP_EVENTHANDLER_H

#include "EpollEvent.h"

namespace netpp {
class EventLoop;
}

namespace netpp::internal::epoll {
// TODO: set handler priority
class EventHandler {
public:
	explicit EventHandler() = default;
	virtual ~EventHandler() = default;

	virtual void handleRead() = 0;
	virtual void handleWrite() = 0;
	virtual void handleError() = 0;
	virtual void handleClose() = 0;

protected:
	EventLoop *_loopThisHandlerLiveIn;
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
