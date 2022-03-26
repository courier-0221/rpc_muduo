#ifndef TIMESTAMP_H
#define TIMESTAMP_H

#include "Common.h"
#include <sys/time.h>
#include <stdio.h>
#define __STDC_FORMAT_MACROS    //保证在inttype.h中将PRId相关宏内容都被定义上
#include <inttypes.h>
#undef __STDC_FORMAT_MACROS
#include <sys/types.h>
#include <string>
#include <iostream>

using namespace std;

class Timestamp : public copyable
{
public:
    Timestamp(int64_t microSeconds = 0.0);
    ~Timestamp();
    bool valid();
    int64_t microSecondsSinceEpoch();
    string toString() const;

    static Timestamp now();
    static Timestamp nowAfter(double seconds);
    static double nowMicroSeconds();
    static const int kMicroSecondsPerSecond = 1000 * 1000; //1s=1000ms*1000us
private:
    int64_t _microSecondsSinceEpoch;    //时间戳 us级别
};
bool operator <(Timestamp l, Timestamp r);
bool operator ==(Timestamp l, Timestamp r);

//时间比较 返回s级别时间
inline double timeDifference(Timestamp high, Timestamp low)
{
    int64_t diff = high.microSecondsSinceEpoch() - low.microSecondsSinceEpoch();
    return static_cast<double>(diff) / Timestamp::kMicroSecondsPerSecond;
}

#endif
