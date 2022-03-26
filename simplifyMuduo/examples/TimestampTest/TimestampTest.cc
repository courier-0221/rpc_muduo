#include "CurrentThread.h"
#include "ICommonCallback.h"
#include "Timestamp.h"

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include <stdio.h>
using namespace std;

//测试引用传值
void passByConstReference(const Timestamp& x)
{
    cout << "passByConstReference: " << x.toString().c_str() << endl;
}

//测试普通传值
void passByValue(Timestamp x)
{
    cout << "passByValue: " << x.toString().c_str() << endl;
}

//测试连续获取多次时间，其中间隔的两个时间差
void benchmark()
{
    const int kNumber = 1000*1000;

    std::vector<Timestamp> stamps;
    stamps.reserve(kNumber);
    for (int i = 0; i < kNumber; ++i)
    {
        stamps.push_back(Timestamp::now());
    }
    printf("head: %s\n", stamps.front().toString().c_str());
    printf("tail: %s\n", stamps.back().toString().c_str());
    printf("tail - head: %f\n", timeDifference(stamps.back(), stamps.front()));

    int increments[100] = { 0 }; //小于100um的各个 时差(1um,2us...) 的次数
    int64_t start = stamps.front().microSecondsSinceEpoch();
    for (int i = 1; i < kNumber; ++i)
    {
        int64_t next = stamps[i].microSecondsSinceEpoch();
        int64_t inc = next - start; //差值 um级别
        start = next;
        if (inc < 0)
        {
            printf("reverse!\n");
        }
        else if (inc < 100)
        {
            ++increments[inc];
        }
        else
        {
            printf("big gap %d\n", static_cast<int>(inc));
        }
    }
    //打印次数
    for (int i = 0; i < 100; ++i)
    {
        printf("%2d: %d\n", i, increments[i]);
    }
}

int main()
{
    cout << "main pid =" << ::getpid() << " tid = " << CurrentThread::tid() << endl;

    Timestamp now(Timestamp::now());
    cout << "now: " << now.toString().c_str() << endl;
    passByValue(now);
    passByConstReference(now);
    benchmark();
    return 0;
}
