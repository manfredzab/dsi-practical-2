#include <stdlib.h>
#include <iostream>

using namespace std;

#include "include/bmtest.h"
#include "include/page.h"
#include "include/bufmgr.h"

int MINIBASE_RESTART_FLAG = 0;

int main (int argc, char **argv)
{
	BMTester tester;
	Status status;

	int bufSize = NUMBUF; 
	minibase_globals = new SystemDefs(status, "MINIBASE.DB", 2000, bufSize, "Clock");

	if (status != OK)
	{
		cerr << "Error initializing Minibase.\n";
		exit(2);
	}

//	Page* pg;
//	int pid;
//	status = MINIBASE_BM->NewPage(pid, pg, 51);

	status = tester.RunTests();

	if (status != OK)
	{
		cout << "Error running buffer manager tests\n";
		minibase_errors.show_errors();
		return 1;
	}

	delete minibase_globals;
	cout << endl;
	return 0;
}
