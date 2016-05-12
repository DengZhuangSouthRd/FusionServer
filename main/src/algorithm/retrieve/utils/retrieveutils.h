#ifndef RETRIEVEUTILS_H
#define RETRIEVEUTILS_H

#include <iostream>
#include <string.h>
#include <map>
#include <string>
#include <fstream>
#include <vector>

#include <hiredis/hiredis.h>
#include <pqxx/pqxx>


using namespace std;
using namespace pqxx;

class PGDB {
public:
    PGDB(string con_info);
    ~PGDB();

public:
    void init();
    void reconnect();
    bool pg_exec_sql(string sql);
    bool pg_fetch_sql(string sql, result& fetch_res);
    bool is_Working();

private:
    string m_con_str;
    connection *p_pg_con;
    bool m_isWorking;
};

class MyRedis {
public:
    MyRedis();
    ~MyRedis();

public:
    void init(map<string, string> argvMap);
    void reconnect();
    void reconnect(map<string, string> argvMap);
    bool is_Working();

private:
    redisContext * m_pRedisContext;
    map<string, string> m_conf;
    bool m_isWorking;
};

string getPGConfInfo(const map<string, string>& argvMap);

#endif // RETRIEVEUTILS_H
