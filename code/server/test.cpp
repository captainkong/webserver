#include "echoserver.h"

using namespace std;

int main(int argc, char const *argv[])
{
    EchoServer server(8, 8010, 1024);
    server.start();
    return 0;
}
