

using namespace std;

#include "CDBMgr.h"

int
main(int argc, char **argv)
{
	CDBMgr::Instance().open();

	CDBMgr::Instance().close();


	return 0;
}

