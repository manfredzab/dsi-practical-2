#include <climits>

#include "../include/lru.h"
#include "../include/frame.h"

int LRU::PickVictim()
{
	int victimFrameIndex = INVALID_FRAME;

	clock_t earliestTimestamp = LONG_MAX;
	for (int i = 0; i < this->numOfBuf; i++)
	{
		Frame& potentialVictim = (*this->frames)[i];
		if (potentialVictim.NotPinned())
		{
			if (potentialVictim.GetTimestamp() < earliestTimestamp)
			{
				earliestTimestamp = potentialVictim.GetTimestamp();
				victimFrameIndex = i;
			}
		}
	}

	return victimFrameIndex;
}


/*
int Clock::PickVictim()
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
*/

