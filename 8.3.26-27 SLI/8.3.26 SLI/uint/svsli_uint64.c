#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

#define ScalarType uint64_t
#define VectorType svuint64_t
//#define Ustype uint64_t
#define Doublelenth __uint128_t

#define WhileLT svwhilelt_b64
#define COUNT svcntd
#define Bitsofs 64 
#define MAX_VALUE UINT64_MAX
#define MIN_VALUE 0

#define imm3 41


static Doublelenth bigrand()//大随机数生成，2**62-1 or 2**93-1
{
    //srand((unsigned)time(NULL));
    Doublelenth bigran;
    switch(sizeof(Doublelenth)/8)
    {
        case 0:
        bigran=rand()%((Doublelenth)1<<Bitsofs);
        break;
        case 1:
        bigran=((Doublelenth)rand()<<31)+rand();
        break;
        default:
        bigran=((Doublelenth)rand()<<62)+(rand()<<31)+rand();
    }
    return bigran;
}

static void calc_vecsli_opt(ScalarType *r,ScalarType *op1,ScalarType *op2,size_t cmputSize,uint64_t imm)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,cmputSize);
        //Load a vector of op1
        VectorType svop1=svld1(pred1,op1+i);
        //Load a vector of op2
        VectorType svop2=svld1(pred1,op2+i);
        //result
        VectorType svr=svsli(svop1,svop2,imm);
        //Store result
        svst1(pred1,r+i,svr);

    } 
}
static void calc_vecsli_ref(ScalarType *out,ScalarType *op1,ScalarType *op2,size_t cmputSize)
{
    for (size_t i=0;i<cmputSize;++i)
    {
        ScalarType temp1=op1[i]-(op1[i]>>imm3<<imm3);//取op1后imm3位
        out[i]=(op2[i]<<imm3)+temp1;
}
}

int test_svsli_uint64(size_t cmputSize)
{
    ScalarType *ref_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *op1=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *op2=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    srand((unsigned)time(NULL));

    int ret=0;
    

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((Doublelenth)1<<Bitsofs)+MIN_VALUE;
        opt_x[i]=bigrand()%((Doublelenth)1<<Bitsofs)+MIN_VALUE;
        op1[i]=bigrand()%((Doublelenth)1<<Bitsofs)+MIN_VALUE;
        op2[i]=bigrand()%((Doublelenth)1<<Bitsofs)+MIN_VALUE;
    }
    uint imm=rand()%Bitsofs;

    calc_vecsli_opt(opt_x,op1,op2,cmputSize,imm);
    calc_vecsli_ref(ref_x,op1,op2,cmputSize);

    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,op1=%llu,op2=%llu,imm3=%llu,ref_x=%llu,opt_x=%llu\n",i,(__uint64_t)op1[i],(__uint64_t)op2[i],imm3,(__uint64_t)ref_x[i],(__uint64_t)opt_x[i]);
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
    free(op1);
    op1=NULL;
    free(op2);
    op2=NULL;

    return ret;
}