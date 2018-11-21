#ifndef LF_NET_CHANNEL
#define LF_NET_CHANNEL

#include <functional>
#include "EventLoop.h"

class Channel
{
	using EventCallback = std::function<void()>;

public:
	Channel(EventLoop* loop, int fd);
	void handleEvent();
	void setReadCallback(const EventCallback& cb);
	void setWriteCallback(const EventCallback& cb);
	void setCloseCallback(const EventCallback& cb);
	void setErrorCallback(const EventCallback& cb);

	void set_revent(int revent);
	void set_index(int index);

	bool isNoneEvent();

	int index();
	int fd();
	int events();
	int revents();

private:
	void update();

	EventLoop* event_loop_;
	const int fd_;
	int events_;
	int revents_;
	int index_;

	EventCallback readCallback_;
	EventCallback writeCallback_;
	EventCallback closeCallback_;
	EventCallback errorCallback_;

	static const int kNoneEvent;
	static const int kErrorEvent;
	static const int kReadEvent;
	static const int kWriteEvent;
};

#endif