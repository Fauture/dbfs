
#include "CDBMgr.h"
#include <direct.h>
#include <string.h>
#include <fstream>

#include "bdb/db_cxx.h"

#ifdef _DEBUG
#pragma comment(lib, "debug/libdb44d.lib")
#else
#pragma comment(lib, "release/libdb181.lib")
#endif // DEBUG

const char *progname = "DTSSEnv";
const char *data_dir = "data";
const char *home = "C:\\Users\\Administrator\\Desktop\\dbfs";
const char * err1 = "DbEnv::open: No such file or directory";
const char * err2 = "Db::open: No such file or directory";

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
			DB_CREATE | DB_INIT_LOCK | DB_INIT_MPOOL , 0);
		//m_pDbEnv->open(home, DB_CREATE, 0);
	}
	catch (DbException &dbe) {
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
	m_pDBAlarm2->cursor(NULL,&dbc,0);
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
			int s=_mkdir(str4.c_str());
			//cout << s << endl;;
		}
		else {
			cout << str4 << " : " << endl;;
			ofstream outFile(str4, ios::out | ios::binary);
			outFile.write((char*)dbValue.get_data(), dbValue.get_size());
			outFile.close();
		}
		delete value;
	}
	return 0;
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
	catch (DbException &dbe) {
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


int CDBMgr::putString(Db* pDb, const char* key, const char* value,int len)
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
		char *key_string = (char *)dbKey.get_data();
		char *data_string = (char *)data.get_data();
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
