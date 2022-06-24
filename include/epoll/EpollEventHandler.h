//
// Created by 17271 on 2022/6/8.
//

#ifndef NETPP_EPOLLEVENTHANDLER_H
#define NETPP_EPOLLEVENTHANDLER_H

#include <cstdint>
#include <memory>

struct epoll_event;

namespace netpp {
class EventLoop;
class Epoll;

/**
 * @brief Supported epoll events
 *
 */
enum EpollEv {
	NOEV = 0,
	/** @brief EPOLLIN event */
	IN		= 0x001,
	/** @brief EPOLLOUT event */
	OUT		= 0x004,
	/** @brief EPOLLRDHUP event */
	RDHUP	= 0x2000,
	/**
	 * @brief EPOLLPRI event
	 * @todo out-of-band data are not handled
	 * */
	PRI		= 0x002,
	/** @brief EPOLLERR event */
	ERR		= 0x008,
	/** @brief EPOLLHUP event */
	HUP		= 0x010
};

class EpollEventHandler {
	friend class Epoll;
public:
	explicit EpollEventHandler(EventLoop *loop);

	virtual ~EpollEventHandler();

	virtual void handleIn() {}

	virtual void handleOut() {}

	virtual void handleErr() {}

	virtual void handleRdhup() {}

	virtual void handlePri() {}

	virtual void handleHup() {}

	/**
	 * @brief Get watching fd of this event
	 * @return file descriptor
	 */
	virtual int fileDescriptor() const = 0;

protected:
	/**
	 * @brief Active watch event
	 * @param event Interested event
	 */
	void activeEvents(std::uint32_t event);

	/**
	 * @brief Deactivate watch event
	 * @param event Event to be deactivated
	 */
	void deactivateEvents(std::uint32_t event);

	void setEvents(std::uint32_t event);

	void disableEvent();

	/**
	 * @brief Called when interested event happened, dispatch event
	 */
	void handleEvents(std::uint32_t events);

	EventLoop *_loopThisHandlerLiveIn;

// for Epoll
private:
	/**
	 * @brief Return current activeEvents epoll events
	 */
	[[nodiscard]] inline ::epoll_event *interestedEvent() const { return m_interestedEvents.get(); }

private:
	bool m_registeredToEpoll;
	Epoll *_poll;
	std::unique_ptr<::epoll_event> m_interestedEvents;
};
}

#endif //NETPP_EPOLLEVENTHANDLER_H
