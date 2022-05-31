#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

#define ScalarType int32_t
#define VectorType svint32_t
#define Doublelenth int64_t

#define WhileLT svwhilelt_b32
#define COUNT svcntw
#define Slrlen 31
#define MAX_VALUE INT32_MAX
#define MIN_VALUE INT32_MIN

Doublelenth bigrand()//大随机数生成，2**62-1 or 2**93-1
{
    srand((unsigned)time(NULL));
    Doublelenth bigran;
    switch(sizeof(Doublelenth)/8)
    {
        case 0:
        bigran=rand()%(2<<Slrlen);
        break;
        case 1:
        bigran=(Doublelenth)(rand()<<31)+rand();
        break;
        default:
        bigran=(Doublelenth)(rand()<<62)+(rand()<<31)+rand();
    }
    return bigran;
}



static void calc_vecmulh_opt(ScalarType *c,ScalarType *a,ScalarType b,size_t cmputSize)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,cmputSize);
        //Load a vector of a
        VectorType sva=svld1(pred1,a+i);
        //mul svc
        VectorType svc=svqrdmulh(sva,b);
        //Store ab
        svst1(pred1,c+i,svc);

    } 
}
static void calc_vecmulh_ref(ScalarType *out,ScalarType *a,ScalarType b,size_t cmputSize)
{
    for (size_t i=0;i<cmputSize;++i)
    {
        Doublelenth temp=(Doublelenth)a[i]*b;
        //out[i]=(temp-1>>Slrlen)+1;向上取整
        out[i]=((temp>>Slrlen-1)+1)/2;//四舍五入
}
}

int test_svqrdmulh_int32_vs(size_t cmputSize)
{
    ScalarType *ref_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *a=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    int ret=0;
    srand((unsigned)time(NULL));
    ScalarType b=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
        opt_x[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
        a[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
    }

    calc_vecmulh_opt(opt_x,a,b,cmputSize);
    calc_vecmulh_ref(ref_x,a,b,cmputSize);

    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,a:%lld,b:%lld,ref_x=%lld,opt_x=%lld\n",i,a[i],b,ref_x[i],opt_x[i]);
            ret=1;

        }
    }

    if(ret==0)
    {
        printf("%s, %d TEST PASSED\n",__func__,__LINE__);
    }

    free(opt_x);
    opt_x=NULL;
    free(ref_x);
    ref_x=NULL;
    free(a);
    a=NULL;

    return ret;
}