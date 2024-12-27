#include "Kqueue.hpp"

/* INSTANCIATE */
Kqueue::Kqueue() {
	log::print("Loading Event Handler...");

	_fd = kqueue();	
	if (_fd == ERROR) {
		throwSysErr("kqueue");
	}

	_timeout.tv_sec = TIMEOUT_SEC;
	_timeout.tv_nsec = 0;

	_que.resize(EVENT_POOL);
}

Kqueue::~Kqueue() {}

int Kqueue::renew() {
	int evnt_new = kevent(_fd, nullptr, 0, _que.data(), _que.size(), nullptr);
	if (evnt_new == ERROR) {
		throwSysErr("kevent_renew");
	}

	return evnt_new;
}

void Kqueue::set(uintptr_t ident, int16_t filter, uint16_t flags, uint32_t fflags, intptr_t data, void* udata) {
	event_t evnt;

	EV_SET(&evnt, ident, filter, flags, fflags, data, udata);
	kevent(_fd, &evnt, 1, nullptr, 0, nullptr);
}

/* ACCESS */
int Kqueue::fd() const { return _fd; }

const event_t& Kqueue::que(const size_t& i) const {
	if (i < _que.size()) {
		return _que[i];
	} else {
		return _que[0];
	}
}

void* Kqueue::castUdata(const int& target) const {
	return reinterpret_cast<void*>(const_cast<int*>(&target));
}

int Kqueue::castUdata(void* target) const {
	if (target == nullptr) {
		std::cout << "udata has set as nullptr\n";
	}

	return *reinterpret_cast<int*>(target);
}