#include "buffer.h"
#include <iostream>

using namespace std;

int main(int argc, char const *argv[])
{
    Buffer buff(4);
    // 读写测试
    size_t len = buff.readFd(STDIN_FILENO, nullptr);
    cout << "读入了" << len << "字节数据" << endl;
    len = buff.writeFd(STDOUT_FILENO, nullptr);
    cout << "写入了" << len << "字节数据" << endl;

    // 其它测试
    string str = "123456789";
    buff.retrieveAll();
    assert(buff.readableBytes() == 0);
    buff.append(str);
    assert(buff.readableBytes() == str.size());

    return 0;
}
