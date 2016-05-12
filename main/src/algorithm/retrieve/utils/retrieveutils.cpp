#include "retrieveutils.h"

#include <iostream>
#include <unistd.h>
#include <stdlib.h>

PGDB::PGDB(string con_info) {
    m_con_str = con_info;
    m_isWorking = false;
    p_pg_con = NULL;
    init();
}

PGDB::~PGDB() {
    if(p_pg_con != NULL && p_pg_con->is_open()) {
        p_pg_con->disconnect();
        delete p_pg_con;
        p_pg_con = NULL;
    }
}

void PGDB::init() {
    try {
        p_pg_con = new connection(m_con_str);
        if(p_pg_con != NULL && p_pg_con->is_open()) {
            cout << "Opened Successful: " << p_pg_con->dbname() << endl;
            m_isWorking = true;
        } else {
            cerr << "Can't Open PostgreSQL !" << endl;
            if(p_pg_con != NULL)
                delete p_pg_con;
            p_pg_con = NULL;
            m_isWorking = false;
        }
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
    }
}

void PGDB::reconnect() {
    if(p_pg_con == NULL) {
        init();
    } else {
        delete p_pg_con;
        p_pg_con = NULL;
        init();
    }
}

bool PGDB::is_Working() {
    return m_isWorking;
}

bool PGDB::pg_exec_sql(string sql) {
    try {
        work W(*p_pg_con);
        W.exec(sql);
        W.commit();
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

bool PGDB::pg_fetch_sql(string sql, result& fetch_res) {
    try {
        nontransaction N(*p_pg_con);
        result R(N.exec(sql));
        fetch_res = R;
    } catch (const std::exception &e) {
        cerr << e.what() << endl;
        return false;
    }
    return true;
}

MyRedis::MyRedis() {
    m_pRedisContext = NULL;
    m_isWorking = false;
}

MyRedis::~MyRedis() {
    if(m_pRedisContext != NULL) {
        redisFree(m_pRedisContext);
        m_pRedisContext = NULL;
    }
}

void MyRedis::init(map<string, string> argvMap) {
    for(map<string, string>::iterator it = argvMap.begin();it != argvMap.end();++it) {
        m_conf[it->first] = it->second;
    }
    if(m_pRedisContext != NULL) {
        redisFree(m_pRedisContext);
        m_pRedisContext = NULL;
    }

    m_pRedisContext = redisConnect(m_conf["host"].c_str(), atoi(m_conf["port"].c_str()));
    if(m_pRedisContext->err) {
        cerr << "Redis Connect Error " << m_pRedisContext->errstr << endl;
        m_isWorking = false;
    }
    m_isWorking = true;
    if(m_conf.find("passwd") != m_conf.end()) {
        string rql = "auth " + m_conf["passwd"];
        redisReply* reply = (redisReply*)redisCommand(m_pRedisContext, rql.c_str());
        cout << "Auth Password Status " << reply->str << ", Str Size is " << reply->len << endl;
        if(strcmp(reply->str, "OK") != 0) {
            cerr << "Redis Auth Error ! Please Check the password !" << endl;
            m_isWorking = false;
        }
        freeReplyObject(reply);
    }
}

void MyRedis::reconnect() {
    init(m_conf);
}

void MyRedis::reconnect(map<string, string> argvMap) {
    init(argvMap);
}

bool MyRedis::is_Working() {
    return m_isWorking;
}

string getPGConfInfo(const map<string, string> &argvMap) {
    string str = "";
    str = "dbname=" + argvMap.at("PG_NAME") + " user=" + argvMap.at("PG_USER") + " password=" + argvMap.at("PG_PASSWD") + " host=" + argvMap.at("PG_HOST") + " port=" + argvMap.at("PG_PORT");
    return str;
}
