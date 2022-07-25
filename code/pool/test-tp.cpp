#include "threadpool.h"
#include <functional>
#include <unistd.h>
#include <stdlib.h>

using namespace std;

void calc_add(int _a, int _b, int *res)
{
    *res = _a + _b;
    cout << this_thread::get_id() << ":" << _a << "+" << _b << "=" << *res << endl;
    sleep(1);
}

int main(int argc, char const *argv[])
{
    // 设置随机种子
    int arr[20][3];
    srand(time(NULL));
    {
        ThreadPool pool(10);
        for (int i = 0; i < 20; ++i)
        {
            int a = rand() % 100, b = rand() % 100;
            pool.addTask(bind(&calc_add, a, b, &arr[i][2]));
            arr[i][0] = a, arr[i][1] = b;
        }
        sleep(2);
    }
    sleep(1);

    for (int i = 0; i < 20; ++i)
    {
        cout << arr[i][0] << "+" << arr[i][1] << "=" << arr[i][2] << endl;
    }

    cout << "done." << endl;
    return 0;
}
