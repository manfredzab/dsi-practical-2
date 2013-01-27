#ifndef FRAME_H
#define FRAME_H

#include <ctime>
#include "page.h"

#define INVALID_FRAME -1

class Frame 
{
	private :
		PageID  pid;
		Page*   data;
		int     pinCount;
		bool    dirty;
		clock_t timestamp;

	public :
		Frame();
		~Frame();
		void    Pin();
		void    Unpin();
		void    EmptyIt();
		void    DirtyIt();
		void    SetPageID(PageID pid);
		bool    IsDirty();
		bool    IsValid();
		Status  Write();
		Status  Read(PageID pid);
		Status  Free();
		bool    NotPinned();
		bool    HasPageID(PageID pid);
		PageID  GetPageID();
		Page*   GetPage();
		clock_t GetTimestamp();
};

#endif
