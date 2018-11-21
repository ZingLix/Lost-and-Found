#ifndef LF_NET_POLLER
#define LF_NET_POLLER
#include <vector>
#include <memory>
#include <map>
#include "Channel.h"
#include <poll.h>

class Poller
{
public:
	using ChannelList = std::vector<Channel*>;
	Poller(EventLoop* loop) :channel_map_(),ownerLoop_(loop) {}

	void updateChannel(Channel*);
	void poll(int, ChannelList* activeChannels);
	void removeChannel(Channel* channel);

private:
	void fillActiveChannels(int num, ChannelList* chs) const;

	using ChannelMap = std::map<int, Channel*>;
	using PollFdList = std::vector<pollfd>;

	ChannelMap channel_map_;
	EventLoop* ownerLoop_;
	PollFdList pollfds_;
};

#endif