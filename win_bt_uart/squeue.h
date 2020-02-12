#ifndef __CSQUEUE_H__
#define __CSQUEUE_H__

#include <queue>
#include <mutex>
#include <chrono>
#include <condition_variable>

template <class T>
class CSQueue {
	public:
		// Add an element to the queue.
		void push(T t) {
			std::lock_guard<std::mutex> lock(m);
			queue_m.lock();
			q.push(t);
			queue_m.unlock();
			c.notify_one();
		}
		T pop(int tout=0) {
			std::unique_lock<std::mutex> lock(m);
			while(q.empty()) {
				if(tout == 0){ c.wait(lock); }
				else {
					if(c.wait_for(lock,std::chrono::milliseconds(tout))==std::cv_status::timeout){
						throw 0;
					}else{
						break;
					}
				}
			}
			queue_m.lock();
			T val = q.front();
			q.pop();
			queue_m.unlock();
			return val;
		}
		inline int size(void){ return q.size(); }
		inline T front(void){ queue_m.lock(); T val = q.front(); q.pop(); queue_m.unlock(); return val; }
		inline void clear(void){ queue_m.lock(); while(q.size())q.pop(); queue_m.unlock(); }
	private:
		std::queue<T> q;
		mutable std::mutex m;
		std::mutex queue_m;
		std::condition_variable c;
};

#endif /* __CSQUEUE_H__ */
