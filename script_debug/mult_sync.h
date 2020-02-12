#ifndef __MULT_SYNC_H__
#define __MULT_SYNC_H__

#include "delay.h"
class CMultSync{
	static int global_cnt;
	int thread_cnt;
	public:
	CMultSync(){ thread_cnt = 0; }
	void pending(void){ thread_cnt++; while(thread_cnt > CMultSync::global_cnt) delay(10); }
	void sync(void){ thread_cnt++; CMultSync::global_cnt++; }
};
int CMultSync::global_cnt=0;
#endif /* __MULT_SYNC_H__ */
