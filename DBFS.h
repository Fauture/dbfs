
#pragma once
#include "bdb/db_cxx.h"
#include <vector>


using namespace std;

class Dbc;
class Db;
class DbEnv;
class DBFS
{
public:
	static DBFS& Instance();

	void open(string name);
	void close();
	void outfile(string& key);


private:
	Db* openDB_0(string name, string dbName);
	Db* openDB(string name, string dbName);
	int closeDB(Db* pDB);

	int getString(Db* pDb, string& key, string& value);

	int getStringAll(Db* pDb, string& key, vector<string>& value);

private:
	DbEnv* m_pDbEnv;
	Db* config;
	Db* file;
	Db* path_i;
};
