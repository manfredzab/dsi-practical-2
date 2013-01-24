#ifndef _LRU_H
#define _LRU_H

#include "frame.h"

class LRU
{
	private:
		int current;
		int numOfBuf;
		Frame** frames;

	public :
		LRU(int n, Frame** f): numOfBuf(n), frames(f), current(0) { };
		int PickVictim();
};

#endif // _LRU_H
