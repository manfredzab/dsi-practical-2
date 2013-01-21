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
}

void Frame::EmptyIt()
{
	this->pid = INVALID_PAGE;
	this->dirty = false;
	this->pinCount = 0;
}

void Frame::DirtyIt()
{
	this->dirty = true;
}

void Frame::SetPageID(PageID pid)
{
	this->pid = pid;
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

	if (this->pinCount <= 1)
	{
		status = MINIBASE_DB->DeallocatePage(this->pid);

		this->EmptyIt();
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
