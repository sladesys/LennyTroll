/*

	This file is part of Lenny Troll project
	Copyright 2020 Slade Systems

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

    	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.

*/

#pragma once

#include <stdio.h>

#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>


//
//
//

class ThreadRunnable {
private:
	uint64_t t;
public:
	inline ThreadRunnable() : t(0) {}
	inline virtual ~ThreadRunnable() {}
	inline void setDelay(const unsigned ms) { t = 0 == ms ?0 :ms + (uint64_t)(std::chrono::steady_clock::now().time_since_epoch().count() / 1000000LL); }
	inline uint64_t getTime() const { return t; }

	virtual void run() = 0;
};

class ThreadQueue {
private:
	std::string name;
	volatile bool started,stopped;

	std::vector<ThreadRunnable*> queue;
	std::mutex mut;
	std::condition_variable notEmpty;

	static void* _thread(void *const arg);

public:
	ThreadQueue(const char *n);

	void start();
	void stop();
	void run();

	void post(ThreadRunnable *const r,const unsigned ms=0);
	ThreadRunnable *pull();
	void cancel(ThreadRunnable *const r);

	inline unsigned size() { return (unsigned)queue.size(); }
	inline bool empty() { return queue.empty(); }
};


