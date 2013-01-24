#include "../include/lru.h"
#include "../include/frame.h"

int LRU::PickVictim()
{

	for (int i = 0; i < 2 * this->numOfBuf; i++) // Do two cycles
	{
		current = (current + i) % this->numOfBuf;

		Frame& potentialVictim = (*this->frames)[current];
		if (potentialVictim.NotPinned())
		{
			if (potentialVictim.IsReferenced())
			{
				potentialVictim.DereferenceIt();
			}
			else
			{
				// Current frame gots to go
				return current;
			}
		}
	}

	return INVALID_FRAME;
}
