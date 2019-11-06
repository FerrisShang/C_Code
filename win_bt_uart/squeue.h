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
			q.push(t);
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
			T val = q.front();
			q.pop();
			return val;
		}
		inline int size(void){ return q.size(); }
		inline T front(void){ return q.front(); }
		inline void clear(void){ while(q.size())q.pop(); }
	private:
		std::queue<T> q;
		mutable std::mutex m;
		std::condition_variable c;
};

#endif /* __CSQUEUE_H__ */
