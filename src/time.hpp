#pragma once

#include <stdint.h>
#include <map>

struct alarm {
	alarm(uint64_t id, double deadline)
	: id_{id}, deadline_{deadline} {}

	void tick(double delta) {
		time_ += delta;
	}

	bool expired() const {
		return time_ >= deadline_;
	}

	uint64_t id() const {
		return id_;
	}

	void rearm() {
		time_ = 0;
	}

private:
	uint64_t id_;
	double deadline_;
	double time_ = 0;
};

struct time_tracker {
	alarm &add_alarm(double deadline) {
		auto id = id_++;
		alarms_.emplace(id, alarm{id, deadline});
		return alarms_.at(id);
	}

	void tick(double delta) {
		for (auto &[_, alarm] : alarms_)
			alarm.tick(delta);
		elapsed_ += delta;
	}

	double now() const {
		return elapsed_;
	}

private:
	uint64_t id_ = 0;
	std::map<uint64_t, alarm> alarms_;
	double elapsed_ = 0;
};
