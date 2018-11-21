#include "Poller.h"


void Poller::poll(int timeout, ChannelList* activeChannels) {
	int num = ::poll(&*pollfds_.begin(), pollfds_.size(), timeout);
	if(num>0) {
		fillActiveChannels(num, activeChannels);
	}
}

void Poller::fillActiveChannels(int num, ChannelList* chs) const {
	for(const auto& pfd:pollfds_) {
		if(pfd.revents>0) {
			--num;
			auto it = channel_map_.find(pfd.fd);
			it->second->set_revent(pfd.revents);
			if (num == 0) break;
		}
	}
}

void Poller::updateChannel(Channel* channel) {
	if (channel->index() < 0){
		pollfd pfd;
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		pollfds_.push_back(pfd);
		int idx = static_cast<int>(pollfds_.size()) - 1;
		channel->set_index(idx);
		channel_map_[pfd.fd] = channel;
	}else{
		int idx = channel->index();
		struct pollfd& pfd = pollfds_[idx];
		pfd.fd = channel->fd();
		pfd.events = static_cast<short>(channel->events());
		pfd.revents = 0;
		if (channel->isNoneEvent()){
			pfd.fd = -channel->fd() - 1;
		}
	}
}

void Poller::removeChannel(Channel* channel) {
	int idx = channel->index();
//	const struct pollfd& pfd = pollfds_[idx];
	channel_map_.erase(channel->fd());
	if (static_cast<size_t>(idx) == pollfds_.size() - 1)
	{
		pollfds_.pop_back();
	}
	else
	{
		int channelAtEnd = pollfds_.back().fd;
		iter_swap(pollfds_.begin() + idx, pollfds_.end() - 1);
		if (channelAtEnd < 0)
		{
			channelAtEnd = -channelAtEnd - 1;
		}
		channel_map_[channelAtEnd]->set_index(idx);
		pollfds_.pop_back();
	}
}