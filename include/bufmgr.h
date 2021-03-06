#ifndef _BUF_H
#define _BUF_H

#include "db.h"
#include "page.h"
#include "frame.h"
#include "lru.h"

class BufMgr 
{
	private:

		Frame* frames;
		LRU*   replacer;
		int    numOfBuf;

		int FindFrame(PageID pid);
		Status FlushFrame(int frameId, bool ignorePinned = false);

		long totalCall;
		long totalMiss;
		long numDirtyPageWrites;

	public:

		BufMgr(int bufsize);
		~BufMgr();      
		Status PinPage(PageID pid, Page*& page, Bool isEmpty = false);
		Status UnpinPage(PageID pid, Bool dirty = false);
		Status NewPage(PageID& pid, Page*& firstpage, int howMany = 1);
		Status FreePage(PageID pid);
		Status FlushPage(PageID pid, bool ignorePinned = false);
		Status FlushAllPages();
		Status GetStat(long& pinNo, long& missNo) { pinNo = totalCall; missNo = totalMiss; return OK; }

		unsigned int GetNumOfUnpinnedFrames();

		void PrintStat();
		void ResetStat() { totalMiss = 0; totalCall = 0; numDirtyPageWrites = 0; }
};


#endif // _BUF_H
