#include "Poller.h"
#include <iostream>


Poller::Poller()
{
}


Poller::~Poller()
{
}

void Poller::addFd(int fd, int events)
{
	struct pollfd pollfd;
	pollfd.fd = fd;
	pollfd.events = events;
	pollfd.revents = 0;

	fdVector.push_back(pollfd);
}

void Poller::removeFd(int fd)
{
	for (int i = 0; i < fdVector.size(); i++) {
		if (fdVector[i].fd == fd) {
			fdVector.erase(fdVector.begin() + i);
			break;
		}
	}
}

void Poller::changeEvents(int fd, int events)
{
	for (int i = 0; i < fdVector.size(); i++) {
		if (fdVector[i].fd == fd) {
			fdVector[i].events = events;
			break;
		}
	}
}

void Poller::_poll()
{
	for (int i = 0; i < fdVector.size(); i++) {
		fdVector[i].revents = 0;
	}

	poll(fdVector.data(), fdVector.size(), -1);
}

std::vector<PollResult> Poller::getPollResult()
{
	std::vector<PollResult> result;
	for (int i = 0; i < fdVector.size(); i++) {
		if (fdVector[i].revents != 0) {
			PollResult pollResult;
			pollResult.fd = fdVector[i].fd;
			pollResult.revents = fdVector[i].revents;

			result.push_back(pollResult);
		}
	}

	return result;
}
