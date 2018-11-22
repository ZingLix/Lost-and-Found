#ifndef LF_NET_EVENTLOOP
#define LF_NET_EVENTLOOP

#include <vector>
#include <memory>
#include <thread>

class Channel;
class Poller;

class EventLoop
{
public:
	EventLoop();
	~EventLoop();

	void loop();
	static EventLoop* getCurrentThreadEventLoop();

	void updateChannel(Channel* ch);
	void removeChannel(Channel* ch);

	static const int kPollTimeMs;

private:
	bool looping_;
	bool quit_;
	std::vector<std::shared_ptr<Channel>> channelList_;
	std::unique_ptr<Poller> poller_;
	const std::thread::id thread_id_;
	std::vector<Channel*> activeList_;
};

#endif