
#pragma once
#include <map>
#include <string>
using namespace std;

class Dbc;
class Db;
class DbEnv;
class CDBMgr
{
public:
	static CDBMgr& Instance();
	CDBMgr(void);
	~CDBMgr(void);

	int open(); //打开数据库环境
	int close();//关闭数据库环境

	string subreplace(string resource_str,string sub_str, string new_str);

private:
	Db* openDB(const char* dbFile); //打开一个数据文件
	Db* openDB1(const char* dbFile); //打开一个数据文件
	Db* openDB2(const char* dbFile); //打开一个数据文件
	int closeDB(Db* pDB);           //关闭一个数据文件
	 
	int del(Db* pDb, const char* key); //写入一个字符串

	int putString(Db* pDb, const char* key, const char* value,int len); //写入一个字符串

	int putString(Db* pDb, const char* key, const char* value); //写入一个字符串
	int getString(Db* pDb, const char* key, string& value);     //读出一个字符串

	int putString(Dbc* pDb, const char* key, const char* value, int len); //写入一个字符串
	int putString(Dbc* pDb, const char* key, const char* value); //写入一个字符串
	int getString(Dbc* pDb, const char* key, string& value);     //读出一个字符串

private:
	DbEnv* m_pDbEnv; 
	Db* m_pDBAlarm1;//报警表1
	Db* m_pDBAlarm2;//报警表2
	Db* m_pDBTemp1; //温度表1 
	Db* m_pDBTemp2; //温度表2
	Db* m_pDBStrain1; //应变表1
	Db* m_pDBStrain2; //应变表2
};
