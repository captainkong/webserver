#include <iostream>
#include <unistd.h>
#include "heaptimer.h"

using namespace std;

void fun1(size_t id)
{
    cout << "done. fd=" << id << endl;
}

int main(int argc, char const *argv[])
{
    HeapTimer ht;
    ht.add(4, 2000, bind(fun1, 4));
    ht.add(2, 3000, bind(fun1, 2));
    ht.add(5, 6000, bind(fun1, 5));
    ht.add(3, 1000, bind(fun1, 3));
    ht.add(1, 100, bind(fun1, 1));
    // ht.display();

    for (int i = 0; i < 5; ++i)
    {
        usleep(1000);
        int time = ht.getNextExpireTime();
        cout << time << endl;
    }

    // ht.display();
    // ht.update(1, 10000);
    // ht.display();

    return 0;
}
