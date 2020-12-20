#pragma once
#include <vector>
#include <sys/poll.h>
#include <sys/types.h>

typedef struct PollResult {
	int fd;
	int revents;
}PollResult;

class Poller
{
private:
	std::vector<pollfd> fdVector;

public:

	Poller();
	~Poller();

	void addFd(int fd, int events);

	void removeFd(int fd);

	void changeEvents(int fd, int events);

	void _poll();

	std::vector<PollResult> getPollResult();

};

