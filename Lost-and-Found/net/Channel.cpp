#include "Channel.h"
#include "poll.h"

const int Channel::kNoneEvent = 0;
const int Channel::kErrorEvent = POLLERR | POLLNVAL;
const int Channel::kReadEvent = POLLIN | POLLPRI;
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
	:event_loop_(loop),fd_(fd),events_(0),revents_(0),index_(-1)
{
}

void Channel::handleEvent() {
	if (revents_&POLLNVAL) {
		//warn
	}
	if (revents_&kErrorEvent) {
		if (errorCallback_) errorCallback_();
	}
	if (revents_&kReadEvent) {
		if (readCallback_) readCallback_();
	}
	if (revents_&kWriteEvent) {
		if (writeCallback_) writeCallback_();
	}
}

void Channel::setReadCallback(const EventCallback& cb) {
	readCallback_ = cb;
}

void Channel::setWriteCallback(const EventCallback& cb) {
	writeCallback_ = cb;
}

void Channel::setCloseCallback(const EventCallback& cb) {
	closeCallback_ = cb;
}

void Channel::setErrorCallback(const EventCallback& cb) {
	errorCallback_ = cb;
}

void Channel::set_revent(int revent) {
	revents_ = revent;
}

void Channel::set_index(int index) { index_ = index; }

bool Channel::isNoneEvent() { return events() == kNoneEvent; }

int Channel::index() {
	return index_;
}

int Channel::fd() { return fd_; }

int Channel::events() { return events_; }

int Channel::revents() { return revents_; }

void Channel::update() {
	event_loop_->updateChannel(this);
}

