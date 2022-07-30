#include "webserver.h"

using namespace std;

int main(int argc, char const *argv[])
{
    WebServer server(8, 8010, 1024);
    server.start();
    return 0;
}
