#include "sqlconnpool.h"
#include <cstring>
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    MYSQL *sqls[10];
    SqlConnPool *sqlPool = SqlConnPool::getInstance();
    sqlPool->init("master", 3306, "root", "Test.2587", "webserver", 10);
    /*
    创建一个测试表
    create table test(
        id int primary key auto_increment,
        msg varchar(32),
        time datetime
    );
    */
    cout << "剩余" << sqlPool->getFreeCount() << "个连接" << endl;
    for (int i = 0; i < 10; ++i)
    {
        cout << "正在获取第" << i + 1 << "个连接...\t";
        sqls[i] = sqlPool->getSqlConnect();
        cout << "剩余" << sqlPool->getFreeCount() << "个连接" << endl;
    }

    for (int i = 0; i < 10; ++i)
    {
        cout << "正在归还第" << i + 1 << "个连接...\t";
        sqlPool->freeSqlConnect(sqls[i]);
        cout << "剩余" << sqlPool->getFreeCount() << "个连接" << endl;
    }

    cout << "SqLSqlRAII测试:" << endl;
    {
        MYSQL *sql = nullptr;
        SqlRAII tem(&sql, sqlPool);
        cout << "剩余" << sqlPool->getFreeCount() << "个连接" << endl;
        // 写测试
        char query[128];
        snprintf(query, 128, "INSERT INTO `test` VALUES(NULL,\'insert test\',now())");
        if (mysql_real_query(sql, query, strlen(query)))
        {
            cout << "数据库写入失败!";
        }

        // 读测试
        memset(query, 0, sizeof(query));
        snprintf(query, 128, "select msg,time from `test` order by id desc limit 1");
        // MYSQL_FIELD *fields = nullptr;
        MYSQL_RES *res = nullptr;

        if (mysql_real_query(sql, query, strlen(query)))
        {
            cout << "查询失败!";
        }

        res = mysql_store_result(sql);
        int n = mysql_num_fields(res);
        int m = res->row_count;
        assert(m == 1 && n == 2);

        while (MYSQL_ROW row = mysql_fetch_row(res))
        {
            cout << "msg:" << row[0] << ",time:" << row[1] << endl;
        }
        // 释放资源
        mysql_free_result(res);
    }

    cout << "剩余" << sqlPool->getFreeCount() << "个连接" << endl;
    return 0;
}
