#include "../include/lru.h"
#include "../include/frame.h"

int LRU::PickVictim()
{
	// TODO: not really LRU

	for (int i = 0; i < this->numOfBuf; i++)
	{
		if ((*this->frames)[i].NotPinned())
		{
			return i;
		}
	}

	return INVALID_FRAME;
}
