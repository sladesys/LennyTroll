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

#include "threader.h"
#include "utils.h"


//
//
//

ThreadQueue::ThreadQueue(const char *n) : name(n),started(false),stopped(false) {}

void ThreadQueue::start() {
	if (started) return;

    std::thread t(_thread,this);
    t.detach();
}

void ThreadQueue::stop() {
	if (!started) return;
	started = false;
	if (stopped) { return; }
	post((ThreadRunnable *)NULL);
	stopped = true;
}

void* ThreadQueue::_thread(void *const arg) {
	ThreadQueue &p = *(ThreadQueue*)arg;
	p.run();
	return NULL;
}

void ThreadQueue::run() {
	threadSetName(name.c_str());
	started = true;

	while (started) {
		ThreadRunnable *const t = pull();
		if (NULL == t) break;

		t->run();
	}
}


//
//
//

void ThreadQueue::post(ThreadRunnable *const in,const unsigned ms) {
	if (stopped) { return; }
	{
		std::lock_guard<std::mutex> l(mut);
		if (0 < ms) in->setDelay(ms);
		queue.push_back(in);
	}
	notEmpty.notify_all();
}

void ThreadQueue::cancel(ThreadRunnable *const r) {
    std::unique_lock<std::mutex> l(mut);

	const int c = (int)queue.size();
	if (0 == c) return;

	for (int i=c-1;0<=i;i--) {
		ThreadRunnable *const rr = queue[(unsigned)i];
		if (NULL == rr) continue;
		if (r != rr) continue;

		queue.erase(queue.begin() +i);
	}
}

ThreadRunnable *ThreadQueue::pull() {
    std::unique_lock<std::mutex> l(mut);

	while (true) {

		const unsigned c = (unsigned)queue.size();

		if (0 < c) {
			int wait = -1;
			const uint64_t now = (uint64_t)(std::chrono::steady_clock::now().time_since_epoch().count() / 1000000LL);

			for (unsigned i=0;i<c;i++) {
				if (NULL == queue[i]) return NULL;

				const uint64_t t = queue[i]->getTime();

				if (0 < t && now < t) {
					wait = (int)std::min((unsigned)wait,(unsigned)(t - now));
					continue;
				}

				ThreadRunnable *const rr = queue[i];
				queue.erase(queue.begin() +i);

				l.unlock();

				return rr;
			}

			if (0 < wait) {
			    notEmpty.wait_for(l,std::chrono::milliseconds(wait));
			    continue;
			}
		}

	    notEmpty.wait(l,[this]() {
	        return !queue.empty();
	    });
	}
}

