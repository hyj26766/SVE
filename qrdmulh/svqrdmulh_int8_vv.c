#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

#define ScalarType int8_t
#define VectorType svint8_t
#define Doublelenth int16_t

#define WhileLT svwhilelt_b8
#define COUNT svcntb
#define Slrlen 7
#define MAX_VALUE INT8_MAX
#define MIN_VALUE INT8_MIN

static Doublelenth bigrand()//大随机数生成，2**62-1 or 2**93-1
{
    //srand((unsigned)time(NULL));
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

static void calc_vecmulh_opt(ScalarType *c,ScalarType *a,ScalarType *b,size_t cmputSize)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,cmputSize);
        //Load a vector of a
        VectorType sva=svld1(pred1,a+i);
        //Load a vector of b
        VectorType svb=svld1(pred1,b+i);
        //Load a vector of c
        VectorType svc=svqrdmulh(sva,svb);
        //Store ab
        svst1(pred1,c+i,svc);

    } 
}
static void calc_vecmulh_ref(ScalarType *out,ScalarType *a,ScalarType *b,size_t cmputSize)
{
    for (size_t i=0;i<cmputSize;++i)
    {
        Doublelenth temp=(Doublelenth)a[i]*(Doublelenth)b[i];
        //out[i]=(temp-1>>Slrlen)+1;向上取整
        out[i]=((temp>>Slrlen-1)+1)/2;//四舍五入
}
}

int test_svqrdmulh_int8_vv(size_t cmputSize)
{
    ScalarType *ref_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *a=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *b=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    int ret=0;
    srand((unsigned)time(NULL));

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((Doublelenth)2<<Slrlen)-MAX_VALUE-1;
        opt_x[i]=bigrand()%((Doublelenth)2<<Slrlen)-MAX_VALUE-1;
        a[i]=bigrand()%((Doublelenth)2<<Slrlen)-MAX_VALUE-1;
        b[i]=bigrand()%((Doublelenth)2<<Slrlen)-MAX_VALUE-1;
    }

    calc_vecmulh_opt(opt_x,a,b,cmputSize);
    calc_vecmulh_ref(ref_x,a,b,cmputSize);

    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,a:%lld,b:%lld,ref_x=%lld,opt_x=%lld\n",i,a[i],b[i],ref_x[i],opt_x[i]);
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
    free(b);
    b=NULL;

    return ret;
}