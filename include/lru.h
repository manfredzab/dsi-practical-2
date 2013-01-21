#ifndef _LRU_H
#define _LRU_H

#include "frame.h"

class LRU
{
	private:
		int numOfBuf;
		Frame** frames;

	public :
		LRU(int n, Frame** f): numOfBuf(n), frames(f) { };
		int PickVictim();
};

#endif // _LRU_H
