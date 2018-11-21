#include "EventLoop.h"
#include "Poller.h"

thread_local EventLoop* thisThreadLoop = nullptr;
const int EventLoop::kPollTimeMs = 2000;

EventLoop::EventLoop()
	:looping_(false),quit_(false),
	poller_(std::make_unique<Poller>(this)), thread_id_(std::this_thread::get_id())
{
	if(thisThreadLoop==nullptr) {
		thisThreadLoop = this;
	}else {
		throw(std::runtime_error("more than one loop this thread"));
	}
}

EventLoop::~EventLoop() {
	thisThreadLoop = nullptr;
}

EventLoop* EventLoop::getCurrentThreadEventLoop() {
	return thisThreadLoop;
}

void EventLoop::loop() {
	looping_ = true;
	quit_ = false;
	while (!quit_) {
		activeList_.clear();
		poller_->poll(kPollTimeMs, &activeList_);
		for (auto it = activeList_.begin(); it != activeList_.end(); ++it) {
			(*it)->handleEvent();
		}
	}
	looping_ = false;
}

void EventLoop::updateChannel(Channel* ch) {
	poller_->updateChannel(ch);
}

void EventLoop::removeChannel(Channel* ch) {
	poller_->removeChannel(ch);
}