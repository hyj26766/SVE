#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>
//#include"imm.h"


#define ScalarType int32_t
#define VectorType svint32_t
#define HalfScalar int16_t
#define HalfVec svint16_t
#define Doublelenth int64_t

#define WhileLT1 svwhilelt_b16
#define WhileLT2 svwhilelt_b32

#define COUNT svcntw

#define MAX_VALUE INT32_MAX
#define MIN_VALUE INT32_MIN
#define MAX_RESULT INT16_MAX
#define MIN_RESULT INT16_MIN

#define imm 5

static Doublelenth bigrand()//大随机数生成，2**62-1 or 2**93-1
{
    //srand((unsigned)time(NULL));
    Doublelenth bigran;
    switch(sizeof(Doublelenth)/8)
    {
        case 0:
        bigran=rand()%((Doublelenth)1<<sizeof(Doublelenth)*4);
        break;
        case 1:
        bigran=((Doublelenth)rand()<<31)+rand();
        break;
        default:
        bigran=((Doublelenth)rand()<<62)+(rand()<<31)+rand();
    }
    return bigran;
}

static void calc_vecshrnb_opt(HalfScalar *r,ScalarType *op1,size_t cmputSize)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred=WhileLT1(i*2,cmputSize*2);
        svbool_t pred1=WhileLT2(i,cmputSize);
        //Load a vector of op1
        VectorType svop1=svld1(pred1,op1+i);
        //result
        HalfScalar svr=svshrnb(svop1,imm);
        //Store result
        svst1(pred1,r+(i*2),svr);

    } 
}
static void calc_vecshrnb_ref(HalfScalar *out,ScalarType *op1,size_t cmputSize)
{
    for (size_t i=0;i<cmputSize;++i)
    {
        ScalarType temp1=op1[i]>>imm;
        //截低位
        out[2*i]=temp1;
        out[2*i+1]=0;
        }
}

int test_svshrnb_int16(size_t cmputSize)
{
    ScalarType *ref_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *op1=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    srand((unsigned)time(NULL));

    int ret=0;
    

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((ScalarType)1<<sizeof(ScalarType)*4)+MIN_RESULT;
        opt_x[i]=bigrand()%((ScalarType)1<<sizeof(ScalarType)*4)+MIN_RESULT;
        op1[i]=bigrand()%((Doublelenth)1<<sizeof(ScalarType)*4)+MIN_VALUE;
    }

    calc_vecshrnb_opt((HalfScalar*)opt_x,op1,cmputSize);
    calc_vecshrnb_ref((HalfScalar*)ref_x,op1,cmputSize);

    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,op1=%lld,op2=%lld,imm=%llu,ref_x=%lld,opt_x=%lld\n",i,(__int64_t)op1[i],(__int64_t)op2[i],imm,(__int64_t)ref_x[i],(__int64_t)opt_x[i]);
            ret=1;

        }
        else
        {
            printf("SUCCESS:%lu,op1=%lld,op2=%lld,imm=%llu,ref_x=%lld,opt_x=%lld\n",i,(__int64_t)op1[i],(__int64_t)op2[i],imm,(__int64_t)ref_x[i],(__int64_t)opt_x[i]);
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
    free(op1);
    op1=NULL;

    return ret;
}