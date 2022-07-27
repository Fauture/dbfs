
#include "CDBMgr.h"
#include <direct.h>
#include <string.h>
#include <fstream>
#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

#include "bdb/db_cxx.h"

#ifdef _DEBUG
#pragma comment(lib, "debug/libdb44d.lib")
#else
#pragma comment(lib, "release/libdb44d.lib")
#endif // DEBUG

const char* progname = "DTSSEnv";
const char* data_dir = "data";
const char* home = "C:\\Users\\Administrator\\Desktop\\dbfs";
const char* err1 = "DbEnv::open: No such file or directory";
const char* err2 = "Db::open: No such file or directory";

//#define	DRTE	"raw"
#define	DRTE "client.dbfs"

#define	NNNN "C:\\Users\\Administrator\\Desktop\\sss"


CDBMgr::CDBMgr(void)
{
	m_pDbEnv = NULL;
}

CDBMgr::~CDBMgr(void)
{
}

CDBMgr& CDBMgr::Instance()
{
	static CDBMgr agent;
	return agent;
}

int CDBMgr::open()
{
	//获取当前目录
	char buffer[256];
	_getcwd(buffer, 256);
	home = buffer;
	if (m_pDbEnv != NULL)
		return -1;

	m_pDbEnv = new DbEnv((u_int32_t)0);
	//m_pDbEnv->set_error_stream(&cerr);
	//m_pDbEnv->set_errpfx(progname);
	//
	// We want to specify the shared memory buffer pool cachesize,
	// but everything else is the default.
	//
	//m_pDbEnv->set_cachesize(0, 64 * 1024, 0);

	// Databases are in a subdirectory.
	(void)m_pDbEnv->set_data_dir(data_dir);

	try {
		//m_pDbEnv->open(home,
		//	DB_CREATE | DB_INIT_REP | DB_INIT_TXN  | DB_INIT_MPOOL | DB_INIT_LOG, 0);

		m_pDbEnv->open(home,
			DB_CREATE | DB_INIT_LOCK | DB_INIT_MPOOL, 0);
		//m_pDbEnv->open(home, DB_CREATE, 0);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << "\n";
		if (!strcmp(dbe.what(), err1)) {
			cout << "Please check whether "
				<< "home dir \"" << home << "\" exists.\n";
		}
		delete m_pDbEnv;
		m_pDbEnv = NULL;
		return -1;
	}

	m_pDBTemp1 = openDB1("config");
	m_pDBAlarm2 = openDB2("path_i");
	m_pDBAlarm1 = openDB("file");


	string value = "";

	Dbc* dbc;
	m_pDBAlarm2->cursor(NULL, &dbc, 0);
	Dbc* dbc2;
	m_pDBAlarm1->cursor(NULL, &dbc2, 0);


	char* key = "";
	//char* value = "";

	Dbt dbKey((char*)key, strlen(key));
	Dbt dbValue;

	//无法获取到数据,但可遍历
	//getString(dbc, "/PlatformDepend/", value);
	//cout << "\r" << endl;;
	//getString(dbc, "/lua/", value);
	//cout << "\r" << endl;;
	//getString(dbc, "/", value);

	//遍历表写出文件
	int ret = 0;
	while ((ret = dbc2->get(&dbKey, &dbValue, DB_NEXT)) == 0) {
		char* value = new char[dbKey.get_size() + 1];
		memcpy(value, dbKey.get_data(), dbKey.get_size());
		value[dbKey.get_size()] = '\0';
		//cout << value << " : " << endl;;

		string str1(home);
		string str2("\\data");
		string str3(value);
		string str4;
		str4 = str1 + str2 + subreplace(str3, "/", "\\");

		if (value[dbKey.get_size() - 1] == 0x2f) {
			int s = _mkdir(str4.c_str());
			//cout << s << endl;;
		}
		else {
			cout << str4 << " : " << endl;;
			ofstream outFile(str4, ios::out | ios::binary);

			IStream* data = SHCreateMemStream((byte*)dbValue.get_data(), dbValue.get_size());

			int type = ReadUInt32(data);//类型
			ReadUInt32(data);//时间戳
			ReadUInt32(data);//时间戳
			int lens = ReadUInt32(data);//解压后大小
			int len = ReadUInt32(data);//解压前大小

			byte* bufferx = (byte*)malloc(lens);
			byte* valuexxs = (byte*)ReadUIntN(data, len);//取出数据 *前面已经取出20字节数据,这里直接取数据无需删除前20字节

			if (type == 5) {
				LDecode(valuexxs, len, bufferx);
				outFile.write((char*)bufferx, lens);
				outFile.close();
			}
			else {
				outFile.write((char*)valuexxs, len);
				outFile.close();
			}

			//outFile.write(valuex, dbValue.get_size());
			//outFile.close();
			free(bufferx);
			free(valuexxs);
			data->Release();
		}
		delete value;
	}
	return 0;
}



char* CDBMgr::ReadUIntN(IStream* pStream, SIZE_T size)
{
	char* imagex = (char*)malloc(size);
	pStream->Read(imagex, size, NULL);
	return imagex;
}

//取出int
int CDBMgr::ReadUInt32(IStream* pStream)
{
	unsigned char buf[4];
	pStream->Read(buf, 4, NULL);
	int ret = ((buf[3] << 24) | (buf[2] << 16) | (buf[1] << 8) | (buf[0] << 0));
	return ret;
}

//取出short
short CDBMgr::ReadUInt16(IStream* pStream)
{
	unsigned char buf[2];
	pStream->Read(buf, 2, NULL);
	short ret = ((buf[1] << 8) | (buf[0] << 0));
	return ret;
}

//取出char
char CDBMgr::ReadUInt8(IStream* pStream)
{
	unsigned char buf[1];
	pStream->Read(buf, 1, NULL);
	char ret = (buf[0] << 0);
	return ret;
}

//跳过字节
void CDBMgr::ReadRun(IStream* pStream, int size)
{
	LARGE_INTEGER dlibMove;
	dlibMove.QuadPart = size;
	//pStream->Seek(dlibMove, STREAM_SEEK_SET, NULL);
	pStream->Seek(dlibMove, STREAM_SEEK_CUR, NULL);
}


int CDBMgr::xx(char* pSrcData)
{
	int A = (unsigned char)*pSrcData++;
	int B = (unsigned char)*pSrcData++;
	int C = (unsigned char)*pSrcData++;
	int D = (unsigned char)*pSrcData++;
	int ret = ((D << 24) | (C << 16) | (B << 8) | (A << 0));

	return ret;
}

int CDBMgr::xxx(char* pSrcData, int x)
{
	for (int i = 0; i < 4 * (x - 1); i++) {
		*pSrcData++;
	}

	int A = (unsigned char)*pSrcData++;
	int B = (unsigned char)*pSrcData++;
	int C = (unsigned char)*pSrcData++;
	int D = (unsigned char)*pSrcData++;

	int ret = ((D << 24) | (C << 16) | (B << 8) | (A << 0));
	return ret;
}


int CDBMgr::LDecode(byte* a2, int xss, byte* a3)
{

	int N = 4096;
	int F = 18;
	int THRESHOLD = 2;

	unsigned char text_buf[4096 + 18 - 1];

	int  i, j, k, r, c;
	unsigned int  flags;

	int ii = 0;
	int iii = 0;
	int ix = 0;

	ix = xss;

	for (i = 0; i < N - F; i++) {
		text_buf[i] = ' ';
	}
	r = N - F;  flags = 0;
	for (; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if (ix == 0) {
				break;
			}
			c = a2[iii++];
			ix -= 1;
			flags = c | 0xff00;
		}
		if (flags & 1) {
			if (ix == 0) {
				break;
			}
			c = a2[iii++];
			ix -= 1;
			a3[ii++] = c;
			text_buf[r++] = c;
			r &= (N - 1);
		}
		else {
			if (ix == 0) {
				break;
			}
			if (ix == 0) {
				break;
			}
			i = a2[iii++];
			j = a2[iii++];
			ix -= 2;
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				a3[ii++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}
	return *a3;
}


/*
 函数说明：对字符串中所有指定的子串进行替换
 参数：
string resource_str            //源字符串
string sub_str                //被替换子串
string new_str                //替换子串
返回值: string
 */
string CDBMgr::subreplace(string resource_str, string sub_str, string new_str)
{
	string dst_str = resource_str;
	string::size_type pos = 0;
	while ((pos = dst_str.find(sub_str)) != string::npos)   //替换所有指定子串
	{
		dst_str.replace(pos, sub_str.length(), new_str);
	}
	return dst_str;
}
int CDBMgr::close()
{
	closeDB(m_pDBAlarm1);
	closeDB(m_pDBAlarm2);
	closeDB(m_pDBTemp1);

	if (m_pDbEnv)
	{
		m_pDbEnv->close(0);
		delete m_pDbEnv;
		m_pDbEnv = NULL;
	}
	return 0;
}

Db* CDBMgr::openDB(const char* dbFile)
{
	// Open a database in the environment to verify the data_dir
	// has been set correctly.
	// Create a database handle, using the environment.	
	Db* pDb = new Db(m_pDbEnv, 0);
	//m_pDB->set_error_stream(&cerr);
	//m_pDB->set_errpfx("AccessExample");
	pDb->set_pagesize(2048);		// Page size: 1K. 
	//m_pDB->set_cachesize(0, 32 * 1024, 0);
	//m_pDB->open(NULL, dbFile, NULL, DB_BTREE, DB_CREATE, 0664);

	pDb->set_flags(0);
	// Open the database. 
	try {
		pDb->open(NULL, DRTE,
			dbFile, DB_BTREE, DB_CREATE, 0664);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << "\n";
		if (!strcmp(dbe.what(), err2)) {
			cout << "Please check whether data dir \"" << data_dir
				<< "\" exists under \"" << home << "\"\n";
		}
		delete pDb;
		pDb = NULL;
		return NULL;
	}

	return pDb;
}

Db* CDBMgr::openDB2(const char* dbFile)
{
	// Open a database in the environment to verify the data_dir
	// has been set correctly.
	// Create a database handle, using the environment.	
	Db* pDb = new Db(m_pDbEnv, 0);
	//m_pDB->set_error_stream(&cerr);
	//m_pDB->set_errpfx("AccessExample");
	pDb->set_pagesize(2048);		// Page size: 1K. 
	//m_pDB->set_cachesize(0, 32 * 1024, 0);
	//m_pDB->open(NULL, dbFile, NULL, DB_BTREE, DB_CREATE, 0664);

	pDb->set_flags(DB_DUPSORT);
	// Open the database. 
	try {
		pDb->open(NULL, DRTE,
			dbFile, DB_BTREE, DB_CREATE, 0664);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << "\n";
		if (!strcmp(dbe.what(), err2)) {
			cout << "Please check whether data dir \"" << data_dir
				<< "\" exists under \"" << home << "\"\n";
		}
		delete pDb;
		pDb = NULL;
		return NULL;
	}

	return pDb;
}

Db* CDBMgr::openDB1(const char* dbFile)
{
	// Open a database in the environment to verify the data_dir
	// has been set correctly.
	// Create a database handle, using the environment.	
	Db* pDb = new Db(m_pDbEnv, 0);
	//m_pDB->set_error_stream(&cerr);
	//m_pDB->set_errpfx("AccessExample");
	pDb->set_pagesize(2048);		// Page size: 1K. 
	//m_pDB->set_cachesize(0, 32 * 1024, 0);
	//m_pDB->open(NULL, dbFile, NULL, DB_BTREE, DB_CREATE, 0664);

	// Open the database. 
	try {
		pDb->open(NULL, DRTE,
			dbFile, DB_HASH, DB_CREATE, 0664);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << "\n";
		if (!strcmp(dbe.what(), err2)) {
			cout << "Please check whether data dir \"" << data_dir
				<< "\" exists under \"" << home << "\"\n";
		}
		delete pDb;
		pDb = NULL;
		return NULL;
	}

	return pDb;
}

int CDBMgr::closeDB(Db* pDB)
{
	if (pDB)
	{
		pDB->close(0);
		delete pDB;
		pDB = NULL;
	}
	return 0;
}



int CDBMgr::del(Db* pDb, const char* key)
{
	Dbt dbKey((unsigned char*)key, strlen(key));

	int ret = -1;
	try {
		ret = pDb->del(0, &dbKey, 0);
	}
	catch (DbException& dbe) {
		cerr << "Exception: " << dbe.what() << "\n";
		return (-1);
	}
	return 0;
}


int CDBMgr::putString(Db* pDb, const char* key, const char* value)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt dbValue((char*)value, strlen(value));
	int ret = pDb->put(0, &dbKey, &dbValue, DB_NOOVERWRITE);
	if (ret == DB_KEYEXIST)
	{
		return -1;
	}
	return 0;
}


int CDBMgr::putString(Db* pDb, const char* key, const char* value, int len)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt dbValue((char*)value, len);
	int ret = pDb->put(0, &dbKey, &dbValue, 0);
	if (ret == DB_KEYEXIST)
	{
		return -1;
	}
	return 0;
}


int CDBMgr::getString(Db* pDb, const char* key, string& value)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt data;
	int ret = pDb->get(NULL, &dbKey, &data, 0);
	if (ret == 0)
	{
		char* key_string = (char*)dbKey.get_data();
		char* data_string = (char*)data.get_data();
		cout << key_string << " : " << data_string << "\n";
		value = (const char*)data.get_data();
		pDb->sync(0);
	}
	else
	{
		char* result = DbEnv::strerror(ret);
		printf(result);
		return -1;
	}

	return 0;
}

int CDBMgr::putString(Dbc* pDb, const char* key, const char* value)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt dbValue((char*)value, strlen(value));
	int ret = pDb->put(&dbKey, &dbValue, DB_NOOVERWRITE);
	if (ret == DB_KEYEXIST)
	{
		return -1;
	}
	return 0;
}

int CDBMgr::putString(Dbc* pDb, const char* key, const char* value, int len)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt dbValue((char*)value, len);
	int ret = pDb->put(&dbKey, &dbValue, DB_KEYFIRST);
	if (ret == DB_KEYEXIST)
	{
		return -1;
	}
	return 0;
}

int CDBMgr::getString(Dbc* pDb, const char* key, string& value)
{
	Dbt dbKey((char*)key, strlen(key));
	Dbt data;
	int ret = pDb->get(&dbKey, &data, DB_SET);
	if (ret == 0)
	{
		char* key_string = (char*)dbKey.get_data();
		char* data_string = (char*)data.get_data();
		cout << key_string << " : " << data_string << "\n";
		value = (const char*)data.get_data();
	}
	else
	{
		char* result = DbEnv::strerror(ret);
		printf(result);
		return -1;
	}

	return 0;
}
