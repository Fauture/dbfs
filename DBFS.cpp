#include "DBFS.h"
#include <fstream>

//����Ѱ�ҷ�debug��
#pragma comment(lib, "debug/libdb44d.lib")

//�����ִ�Сд���ڴ�Ƚϣ��Ӻ�����
int case_insensitive_memcmp(const void* ptr1, const void* ptr2, size_t count) {
	const unsigned char* p1 = static_cast<const unsigned char*>(ptr1);
	const unsigned char* p2 = static_cast<const unsigned char*>(ptr2);

	for (size_t i = 0; i < count; ++i) {
		int c1 = std::tolower(p1[i]);
		int c2 = std::tolower(p2[i]);
		if (c1 != c2) {
			return c1 - c2; // ���ص�һ����ƥ���ֽڵĲ�ֵ
		}
	}
	return 0;
}

// �������Կͻ�����DbfsUp.exe   �����ȱȽϵ��ڴ��Ƚϣ���������
int compare_blocks_ignore_case(
	Db* unused,                  // ����������δʹ�ã�
	const Dbt* block1,  // ���ݿ�1 {ָ��, ����}
	const Dbt* block2   // ���ݿ�2 {ָ��, ����}
) {
	const void* data1 = block1->get_data();
	size_t len1 = block1->get_size();

	const void* data2 = block2->get_data();
	size_t len2 = block2->get_size();

	// ȡ��С���Ƚ��бȽ�
	size_t min_len = (len1 <= len2) ? len1 : len2;

	// �ȽϹ�ͬ���Ȳ��ֵ����ݣ������ִ�Сд��
	int cmp_result = case_insensitive_memcmp(data1, data2, min_len);

	if (cmp_result == 0) {
		// ������ͬ�򷵻س��Ȳ�
		return static_cast<int>(len1 - len2);
	}

	// ���ݲ�ͬ��ֱ�ӷ��رȽϽ��
	return cmp_result;
}

DBFS& DBFS::Instance()
{
	static DBFS agent;
	return agent;
}

int DBFS::open(string name)
{
	if (m_pDbEnv != NULL)
		return -1;
	m_pDbEnv = new DbEnv((u_int32_t)0);
	try {
		m_pDbEnv->open(NULL, DB_CREATE | DB_INIT_MPOOL | DB_PRIVATE | DB_EXCL, 0);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << endl;
		delete m_pDbEnv;
		m_pDbEnv = NULL;
		return -1;
	}

	config = openDB_0(name, "config");
	path_i = openDB(name, "path_i");
	file = openDB(name, "file");


	//false  true
	bool ���� = false;
	bool �ļ� = false;

	if (�ļ�) {
		string key = "/PlatformDepend/diff_jd";
		string value = "";
		int ret = getString(file, key, value);
		if (ret == 0) {
			cout << "key: " << key << " value:  " << endl;
			uint32_t previewSize = std::min<uint32_t>(20, value.size());
			for (uint32_t i = 0; i < previewSize; ++i) {
				printf("%02X ", static_cast<uint8_t>(value.data()[i]));
			}
			size_t lastSlashPos = key.rfind('/');
			if (lastSlashPos != std::string::npos && lastSlashPos < key.size() - 1) {
				string fileName = key.substr(lastSlashPos + 1);
				ofstream outFile("./data/" + fileName, ios::out | ios::binary);
				outFile.write((char*)value.data(), value.size());
				outFile.close();
			}
		}
	}
	else {
		if (����) {
			string key = "/";
			string value = "";
			int ret = getString(path_i, key, value);
			if (ret == 0) {
				cout << "key: " << key << " value:  " << value << endl;
			}
		}
		else {
			string key = "/PlatformDepend/";
			vector<string> value;
			int ret = getStringAll(path_i, key, value);
			if (ret == 0) {
				for (const auto& str : value) {
					cout << "key: " << key << " value:  " << str << endl;
				}
			}

		}
	}

	return 0;
}

int DBFS::close()
{
	closeDB(config);
	closeDB(file);
	closeDB(path_i);

	if (m_pDbEnv)
	{
		m_pDbEnv->close(0);
		delete m_pDbEnv;
		m_pDbEnv = NULL;
	}
	return 0;
}

Db* DBFS::openDB(string name, string dbName)
{
	Db* pDb = new Db(m_pDbEnv, 0);
	pDb->set_pagesize(2048);
	//pDb->set_flags(DB_DUPSORT);
	pDb->set_bt_compare(&compare_blocks_ignore_case);
	try {
		pDb->open(NULL, name.c_str(), dbName.c_str(), DB_BTREE, DB_CREATE, 0664);
	}
	catch (DbException& dbe) {
		cerr << dbName << " : " << dbe.what() << endl;
		delete pDb;
		pDb = NULL;
		return NULL;
	}

	return pDb;
}


Db* DBFS::openDB_0(string name, string dbName)
{
	Db* pDb = new Db(m_pDbEnv, 0);
	pDb->set_pagesize(2048);
	try {
		pDb->open(NULL, name.c_str(), dbName.c_str(), DB_HASH, DB_CREATE, 0664);
	}
	catch (DbException& dbe) {
		cerr << dbName << " : " << dbe.what() << endl;
		delete pDb;
		pDb = NULL;
		return NULL;
	}

	return pDb;
}


int DBFS::closeDB(Db* pDB)
{
	if (pDB)
	{
		pDB->close(0);
		delete pDB;
		pDB = NULL;
	}
	return 0;
}



int DBFS::getString(Db* pDb, string& key, string& value)
{
	Dbt dbKey((char*)key.c_str(), key.size());
	Dbt data;
	int ret = pDb->get(NULL, &dbKey, &data, 0);
	if (ret == 0)
	{
		value.assign((char*)data.get_data(), data.get_size());
	}
	else
	{
		cerr << DbEnv::strerror(ret) << endl;
		return -1;
	}

	return 0;
}


int DBFS::getStringAll(Db* pDb, string& key, vector<string>& value)
{
	Dbt dbKey((char*)key.c_str(), key.size());
	Dbt data;

	Dbc* dbc;
	pDb->cursor(NULL, &dbc, 0);
	int ret = dbc->get(&dbKey, &data, DB_SET);
	if (ret == DB_NOTFOUND) {
		cerr << DbEnv::strerror(ret) << endl;
		return -1;
	}
	else {
		do {
			value.push_back(string((char*)data.get_data(), data.get_size()));
			ret = dbc->get(&dbKey, &data, DB_NEXT_DUP);
		} while (ret == 0);
	}
	dbc->close();

	return 0;
}