#include <stdio.h>
#include <math.h>
#include<stdlib.h>
#include<time.h>
#include<pthread_time.h>

__int32 fun1(__int32 a,__int32 b)
{
    //__int64 temp=(__int64)a+b;
    __int32 result;
    if((__int64)a+b<0)
    {
        result=-1;
    }
    else{
        result=0;
    }
    printf("fun1:  a=%d    ,b=%d     ,result=%d\n",a,b,result);
    return result;
}

__int32 fun2(__int32 a,__int32 b)
{
    __int32 result;
    if(a>=0&&b>=0)
    {
        result=0;
    }
    else if (a<0&&b<0)
    {
        result=-1;
    }
    else{
        if((a+b)<0)
        {
            result=-1;
        }
        else{
            result=0;
        }
    }
    printf("fun2:  a=%d    ,b=%d     ,result=%d\n",a,b,result);
    return result;
}

int main()
{
    srand((unsigned)time(NULL));
    struct timespec time1={0,0},time2={0,0},time3={0,0};
    int cputimes1,cputimes2;
    int cputimens1,cputimens2;
    clock_gettime(CLOCK_REALTIME,&time1);
    for(int i=0;i<10000;i++)
    {
        int a=rand()-16384;
        int b=rand()-16384;
        fun1(a,b);

    }
    clock_gettime(CLOCK_REALTIME,&time2);
    for(int i=0;i<10000;i++)
    {
        int a=rand()-16384;
        int b=rand()-16384;
        fun2(a,b);
    }
    clock_gettime(CLOCK_REALTIME,&time3);

    cputimes1=time2.tv_sec-time1.tv_sec;
    cputimes2=time3.tv_sec-time2.tv_sec;

    cputimens1=time2.tv_nsec-time1.tv_nsec;
    cputimens2=time3.tv_nsec-time2.tv_nsec;

    printf("fun1result time: %1ld ns\n",(__int64)cputimes1*(__int64)pow(10,9)+(__int64)cputimens1);
    printf("fun2result time: %lld ns\n",(__int64)cputimes2*(__int64)pow(10,9)+(__int64)cputimens2);
    return 1;
}