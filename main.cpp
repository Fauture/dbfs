
#include "DBFS.h"

int main(int argc, char **argv)
{
	//自行准备文件至data目录
	DBFS::Instance().open("./data/char.dbfs");

	DBFS::Instance().close();


	return 0;
}

