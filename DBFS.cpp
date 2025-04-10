#include "DBFS.h"
#include <fstream>

//自行寻找非debug库
#pragma comment(lib, "debug/libdb44d.lib")

//不区分大小写的内存比较（子函数）
int case_insensitive_memcmp(const void* ptr1, const void* ptr2, size_t count) {
	const unsigned char* p1 = static_cast<const unsigned char*>(ptr1);
	const unsigned char* p2 = static_cast<const unsigned char*>(ptr2);

	for (size_t i = 0; i < count; ++i) {
		int c1 = std::tolower(p1[i]);
		int c2 = std::tolower(p2[i]);
		if (c1 != c2) {
			return c1 - c2; // 返回第一个不匹配字节的差值
		}
	}
	return 0;
}

// 反编译自客户端下DbfsUp.exe   带长度比较的内存块比较（主函数）
int compare_blocks_ignore_case(
	Db* unused,                  // 保留参数（未使用）
	const Dbt* block1,  // 数据块1 {指针, 长度}
	const Dbt* block2   // 数据块2 {指针, 长度}
) {
	const void* data1 = block1->get_data();
	size_t len1 = block1->get_size();

	const void* data2 = block2->get_data();
	size_t len2 = block2->get_size();

	// 取较小长度进行比较
	size_t min_len = (len1 <= len2) ? len1 : len2;

	// 比较共同长度部分的内容（不区分大小写）
	int cmp_result = case_insensitive_memcmp(data1, data2, min_len);

	if (cmp_result == 0) {
		// 内容相同则返回长度差
		return static_cast<int>(len1 - len2);
	}

	// 内容不同则直接返回比较结果
	return cmp_result;
}

void LDecode(const uint8_t* buf, int len, uint8_t* out)
{
	int N = 4096;
	int F = 18;
	int THRESHOLD = 2;

	uint8_t text_buf[4096 + 18 - 1];

	int  i, j, k, r, c;
	uint32_t  flags;

	int ii = 0;
	int iii = 0;
	int ix = 0;

	ix = len;

	for (i = 0; i < N - F; i++) {
		text_buf[i] = ' ';
	}
	r = N - F;  flags = 0;
	for (; ; ) {
		if (((flags >>= 1) & 256) == 0) {
			if (ix == 0) {
				break;
			}
			c = buf[iii++];
			ix -= 1;
			flags = c | 0xff00;
		}
		if (flags & 1) {
			if (ix == 0) {
				break;
			}
			c = buf[iii++];
			ix -= 1;
			out[ii++] = c;
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
			i = buf[iii++];
			j = buf[iii++];
			ix -= 2;
			i |= ((j & 0xf0) << 4);
			j = (j & 0x0f) + THRESHOLD;
			for (k = 0; k <= j; k++) {
				c = text_buf[(i + k) & (N - 1)];
				out[ii++] = c;
				text_buf[r++] = c;
				r &= (N - 1);
			}
		}
	}
}

uint32_t read_le_u32(const uint8_t* p) {
	return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}


DBFS& DBFS::Instance()
{
	static DBFS agent;
	return agent;
}

void DBFS::open(string name)
{
	if (m_pDbEnv != NULL)
		return;
	m_pDbEnv = new DbEnv((u_int32_t)0);
	try {
		m_pDbEnv->open(NULL, DB_CREATE | DB_INIT_MPOOL | DB_PRIVATE | DB_EXCL, 0);
	}
	catch (DbException& dbe) {
		cerr << "EnvExample: " << dbe.what() << endl;
		delete m_pDbEnv;
		m_pDbEnv = NULL;
		return;
	}

	config = openDB_0(name, "config");
	path_i = openDB(name, "path_i");
	file = openDB(name, "file");


	//false  true
	bool 单个 = false;
	bool 文件 = true;

	if (文件) {
		string key = "/PlatformDepend/diff_jd";
		outfile(key);
	}
	else {
		if (单个) {
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
}

void DBFS::close()
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
		dbc->close();
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


void DBFS::outfile(string& key) {
	string value = "";
	int ret = getString(file, key, value);
	if (ret == 0) {
		cout << "key: " << key << " value:  ";
		uint32_t previewSize = std::min<uint32_t>(20, value.size());
		for (uint32_t i = 0; i < previewSize; ++i) {
			printf("%02X ", static_cast<uint8_t>(value.data()[i]));
		}
		cout << endl;

		size_t lastSlashPos = key.rfind('/');
		if (lastSlashPos != std::string::npos && lastSlashPos < key.size() - 1) {
			string fileName = key.substr(lastSlashPos + 1);

			const size_t HEADER_SIZE = 20;
			if (value.size() < HEADER_SIZE)
			{
				std::cerr << "数据不够" << std::endl;
				return;
			}

			const uint8_t* ptr = (uint8_t*)value.data();

			uint32_t type = read_le_u32(ptr); ptr += 4;
			uint32_t utime = read_le_u32(ptr); ptr += 4;
			uint32_t time = read_le_u32(ptr); ptr += 4;
			uint32_t usize = read_le_u32(ptr); ptr += 4;
			uint32_t size = read_le_u32(ptr); ptr += 4;

			std::cout << "type: " << type << "\n"
				<< "utime: " << utime << "\n"
				<< "time: " << time << "\n"
				<< "usize: " << usize << "\n"
				<< "size: " << size << std::endl;

			if (value.size() < HEADER_SIZE + size) {
				std::cerr << "数据长度错误" << std::endl;
				return;
			}
			if (usize == 0) {
				std::cerr << "文件长度错误" << std::endl;
				return;
			}
			uint8_t* databuff = (uint8_t*)malloc(usize);

			if (type == 5) {
				LDecode((uint8_t*)ptr, size, databuff);
			}
			else {
				if (databuff != nullptr) {
					memcpy(databuff, ptr, usize);
				}
				else {
					std::cerr << "内存为空" << std::endl;
					return;
				}
			}
			ofstream outFile("./data/" + fileName, ios::out | ios::binary);
			outFile.write((char*)databuff, usize);
			outFile.close();

			free(databuff);
			databuff = NULL;
		}
	}

}

