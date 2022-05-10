
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

	int open(); //�����ݿ⻷��
	int close();//�ر����ݿ⻷��

	string subreplace(string resource_str,string sub_str, string new_str);

private:
	Db* openDB(const char* dbFile); //��һ�������ļ�
	Db* openDB1(const char* dbFile); //��һ�������ļ�
	Db* openDB2(const char* dbFile); //��һ�������ļ�
	int closeDB(Db* pDB);           //�ر�һ�������ļ�
	 
	int del(Db* pDb, const char* key); //д��һ���ַ���

	int putString(Db* pDb, const char* key, const char* value,int len); //д��һ���ַ���

	int putString(Db* pDb, const char* key, const char* value); //д��һ���ַ���
	int getString(Db* pDb, const char* key, string& value);     //����һ���ַ���

	int putString(Dbc* pDb, const char* key, const char* value, int len); //д��һ���ַ���
	int putString(Dbc* pDb, const char* key, const char* value); //д��һ���ַ���
	int getString(Dbc* pDb, const char* key, string& value);     //����һ���ַ���

private:
	DbEnv* m_pDbEnv; 
	Db* m_pDBAlarm1;//������1
	Db* m_pDBAlarm2;//������2
	Db* m_pDBTemp1; //�¶ȱ�1 
	Db* m_pDBTemp2; //�¶ȱ�2
	Db* m_pDBStrain1; //Ӧ���1
	Db* m_pDBStrain2; //Ӧ���2
};
