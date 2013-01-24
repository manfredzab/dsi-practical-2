#include "../include/frame.h"
#include "../include/db.h"

Frame::Frame()
{
	this->data = new Page();
	this->EmptyIt();
}

Frame::~Frame()
{
	// Flush the frame to disk if dirty
	if (this->IsDirty())
	{
		this->Write();
	}

	delete this->data;
}

void Frame::Pin()
{
	this->pinCount++;
}

void Frame::Unpin()
{
	this->pinCount--;

	if (0 == this->pinCount)
	{
		this->referenced = true;
	}
}

void Frame::EmptyIt()
{
	this->pid = INVALID_PAGE;
	this->dirty = false;
	this->referenced = false;
	this->pinCount = 0;
}

void Frame::DereferenceIt()
{
	this->referenced = false;
}

void Frame::DirtyIt()
{
	this->dirty = true;
}

void Frame::SetPageID(PageID pid)
{
	this->pid = pid;
}

bool Frame::IsReferenced()
{
	return this->referenced;
}

bool Frame::IsDirty()
{
	return this->dirty;
}

bool Frame::IsValid()
{
	return (this->pid != INVALID_PAGE);
}

Status Frame::Write()
{
	return MINIBASE_DB->WritePage(this->pid, this->data);
}

Status Frame::Read(PageID pid)
{
	Status status = MINIBASE_DB->ReadPage(pid, data);

	if (OK == status)
	{
		this->pid = pid;
	}

	return status;
}

Status Frame::Free()
{
	Status status = OK;

	if (this->pinCount > 1)
	{
		status = FAIL;
	}

	if (OK == status)
	{
		PageID pid = this->pid;

		// Release the frame (the space might be needed by the directory page)
		this->EmptyIt();

		status = MINIBASE_DB->DeallocatePage(pid);
	}

	return status;
}

bool Frame::NotPinned()
{
	return (this->pinCount == 0);
}

bool Frame::HasPageID(PageID pid)
{
	return (this->pid == pid);
}

PageID Frame::GetPageID()
{
	return this->pid;
}

Page* Frame::GetPage()
{
	return this->data;
}
