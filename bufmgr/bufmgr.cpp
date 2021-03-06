
#include "../include/bufmgr.h"
#include "../include/frame.h"
#include "../include/lru.h"
#include "../include/db.h"

//--------------------------------------------------------------------
// Constructor for BufMgr
//
// Input   : bufSize  - number of pages in the this buffer manager
// Output  : None
// PostCond: All frames are empty.
//--------------------------------------------------------------------

BufMgr::BufMgr(int bufSize)
{
	this->numOfBuf = bufSize;
	this->frames = new Frame[bufSize];

	this->replacer = new LRU(this->numOfBuf, &this->frames);

	this->ResetStat();
}


//--------------------------------------------------------------------
// Destructor for BufMgr
//
// Input   : None
// Output  : None
//--------------------------------------------------------------------

BufMgr::~BufMgr()
{
	delete this->replacer;

	// Frame destructor is responsible for flushing the frame to disk if it was dirty.
	delete[] this->frames;
}

//--------------------------------------------------------------------
// BufMgr::PinPage
//
// Input    : pid     - page id of a particular page 
//            isEmpty - (optional, default to false) if true indicate
//                      that the page to be pinned is an empty page.
// Output   : page - a pointer to a page in the buffer pool. (NULL
//            if fail)
// Purpose  : Pin the page with page id = pid to the buffer.  
//            Read the page from disk unless isEmpty is true or unless
//            the page is already in the buffer.
// Condition: Either the page is already in the buffer, or there is at
//            least one frame available in the buffer pool for the 
//            page.
// PostCond : The page with page id = pid resides in the buffer and 
//            is pinned. The number of pin on the page increase by
//            one.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------


Status BufMgr::PinPage(PageID pid, Page*& page, bool isEmpty)
{
	// Collect stats
	totalCall++;

	Status status = OK;
	page = NULL;

	// Check if the buffer contains the page "pid"
	int frameId = this->FindFrame(pid);

	if (INVALID_FRAME == frameId)
	{
		totalMiss++;

		// Find a victim
		frameId = replacer->PickVictim();

		if (INVALID_FRAME == frameId)
		{
			status = FAIL;
		}

		if (OK == status)
		{
			Frame& victimFrame = this->frames[frameId];
			this->FlushFrame(frameId);

			if (OK == status)
			{
				if (isEmpty)
				{
					victimFrame.SetPageID(pid);
				}
				else
				{
					status = victimFrame.Read(pid);
				}
			}
		}
	}

	if (OK == status)
	{
		// Increment the pin count
		this->frames[frameId].Pin();

		// Set the return value for page
		page = this->frames[frameId].GetPage();
	}

	return status;
} 

//--------------------------------------------------------------------
// BufMgr::UnpinPage
//
// Input    : pid     - page id of a particular page 
//            dirty   - indicate whether the page with page id = pid
//                      is dirty or not. (Optional, default to false)
// Output   : None
// Purpose  : Unpin the page with page id = pid in the buffer. Mark 
//            the page dirty if dirty is true.  
// Condition: The page is already in the buffer and is pinned.
// PostCond : The page is unpinned and the number of pin on the
//            page decrease by one. 
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------


Status BufMgr::UnpinPage(PageID pid, bool dirty)
{
	Status status = OK;

	int frameId = this->FindFrame(pid);
	if (INVALID_FRAME == frameId)
	{
		status = FAIL;
	}

	if (OK == status)
	{
		this->frames[frameId].Unpin();

		if (dirty)
		{
			this->frames[frameId].DirtyIt();
		}
	}

	return status;
}

//--------------------------------------------------------------------
// BufMgr::NewPage
//
// Input    : howMany - (optional, default to 1) how many pages to 
//                      allocate.
// Output   : firstPid  - the page id of the first page (as output by
//                   DB::AllocatePage) allocated.
//            firstPage - a pointer to the page in memory.
// Purpose  : Allocate howMany number of pages, and pin the first page
//            into the buffer. 
// Condition: howMany > 0 and there is at least one free buffer space
//            to hold a page.
// PostCond : The page with page id = pid is pinned into the buffer.
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call DB::AllocatePage() to allocate a page.  
//            You should call DB:DeallocatePage() to deallocate the
//            pages you allocate if you failed to pin the page in the
//            buffer.
//--------------------------------------------------------------------


Status BufMgr::NewPage (PageID& firstPid, Page*& firstPage, int howMany)
{
	Status status = OK;

	if (howMany < 1)
	{
		status = FAIL;
	}

	if (OK == status)
	{
		status = MINIBASE_DB->AllocatePage(firstPid, howMany);
	}

	if (OK == status)
	{
		status = this->PinPage(firstPid, firstPage, true);

		if (status != OK)
		{
			status = MINIBASE_DB->DeallocatePage(firstPid, howMany);
		}
	}

	return status;
}

//--------------------------------------------------------------------
// BufMgr::FreePage
//
// Input    : pid     - page id of a particular page 
// Output   : None
// Purpose  : Free the memory allocated for the page with 
//            page id = pid  
// Condition: Either the page is already in the buffer and is pinned
//            no more than once, or the page is not in the buffer.
// PostCond : The page is unpinned, and the frame where it resides in
//            the buffer pool is freed.  Also the page is deallocated
//            from the database. 
// Return   : OK if operation is successful.  FAIL otherwise.
// Note     : You can call MINIBASE_DB->DeallocatePage(pid) to
//            deallocate a page.
//--------------------------------------------------------------------


Status BufMgr::FreePage(PageID pid)
{
	Status status = OK;

	int frameId = this->FindFrame(pid);
	if (INVALID_FRAME != frameId)
	{
		status = this->frames[frameId].Free();
	}
	else
	{
		status = MINIBASE_DB->DeallocatePage(pid); // Deallocate the page even if it was not in the buffer
	}

	return status;
}


//--------------------------------------------------------------------
// BufMgr::FlushPage
//
// Input    : pid  - page id of a particular page 
// Output   : None
// Purpose  : Flush the page with the given pid to disk.
// Condition: The page with page id = pid must be in the buffer,
//            and is not pinned. pid cannot be INVALID_PAGE.
// PostCond : The page with page id = pid is written to disk if it's dirty. 
//            The frame where the page resides is empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------


Status BufMgr::FlushPage(PageID pid, bool ignorePinned)
{
	Status status = OK;

	if (INVALID_PAGE == pid)
	{
		status = FAIL;
	}

	int frameId = INVALID_FRAME;
	if (OK == status)
	{
		frameId = this->FindFrame(pid);

		if (INVALID_FRAME == frameId)
		{
			status = FAIL;
		}
	}

	status = this->FlushFrame(frameId, ignorePinned);

	return status;
}




//--------------------------------------------------------------------
// BufMgr::FlushAllPages
//
// Input    : None
// Output   : None
// Purpose  : Flush all pages in this buffer pool to disk.
// Condition: All pages in the buffer pool must not be pinned.
// PostCond : All dirty pages in the buffer pool are written to 
//            disk (even if some pages are pinned). All frames are empty.
// Return   : OK if operation is successful.  FAIL otherwise.
//--------------------------------------------------------------------

Status BufMgr::FlushAllPages()
{
	bool success = true;
	for (int i = 0; i < this->numOfBuf; i++)
	{
		if (this->frames[i].IsValid())
		{
			success &= this->frames[i].NotPinned();
			success &= (this->FlushFrame(i, true) == OK);

			frames[i].EmptyIt();
		}
	}

	return success ? OK : FAIL;
}


//--------------------------------------------------------------------
// BufMgr::GetNumOfUnpinnedFrames
//
// Input    : None
// Output   : None
// Purpose  : Find out how many unpinned locations are in the buffer
//            pool.
// Condition: None
// PostCond : None
// Return   : The number of unpinned buffers in the buffer pool.
//--------------------------------------------------------------------

unsigned int BufMgr::GetNumOfUnpinnedFrames()
{
	int numOfUnpinnedFrames = 0;

	for (int i = 0; i < this->numOfBuf; i++)
	{
		// Used only for test case; does not really make sense (see bmtest.cpp, Test2).
		if (!this->frames[i].IsValid() || this->frames[i].NotPinned())
		{
			numOfUnpinnedFrames++;
		}
	}

	return numOfUnpinnedFrames;
}


void  BufMgr::PrintStat() {
	cout << "**Buffer Manager Statistics**" << endl;
	cout << "Number of Dirty Pages Written to Disk: " << numDirtyPageWrites << endl;
	cout << "Number of Pin Page Requests: " << totalCall << endl;
	cout << "Number of Pin Page Request Misses " << totalMiss << endl;
}

//--------------------------------------------------------------------
// BufMgr::FindFrame
//
// Input    : pid - a page id 
// Output   : None
// Purpose  : Look for the page in the buffer pool, return the frame
//            number if found.
// PreCond  : None
// PostCond : None
// Return   : the frame number if found. INVALID_FRAME otherwise.
//--------------------------------------------------------------------

int BufMgr::FindFrame(PageID pid)
{
	for (int i = 0; i < this->numOfBuf; i++)
	{
		if (this->frames[i].GetPageID() == pid)
		{
			return i;
		}
	}

	return INVALID_FRAME;
}


Status BufMgr::FlushFrame(int frameId, bool ignorePinned)
{
	Status status = OK;

	// If we care about flushing pinned pages and the frame is pinned - fail.
	if (OK == status)
	{
		if (!ignorePinned && !this->frames[frameId].NotPinned())
		{
			status = FAIL;
		}
	}

	// Flush the frame to disk
	if (OK == status)
	{
		Frame& frame = this->frames[frameId];

		if (frame.IsDirty())
		{
			// Collect stats
			numDirtyPageWrites++;

			status = frame.Write();
		}

		frame.EmptyIt();
	}

	return status;
}
