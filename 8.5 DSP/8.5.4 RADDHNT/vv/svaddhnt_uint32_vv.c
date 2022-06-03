#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

#define ScalarType uint32_t
#define VectorType svuint32_t
#define HalfScalar uint16_t
#define HalfVec svuint16_t
#define Doublelenth uint64_t

#define WhileLT svwhilelt_b32
#define COUNT svcntw

#define MAX_VALUE UINT32_MAX
#define MIN_VALUE 0
#define MAX_RESULT UINT16_MAX
#define MIN_RESULT 0

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

static void calc_vecaddhnt_opt(HalfScalar *r,HalfScalar *even,ScalarType *op1,ScalarType *op2,size_t cmputSize)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,cmputSize);
        //Load a vector of even
        HalfVec sveven=svld1(pred1,even+i);
        //Load a vector of op1
        VectorType svop1=svld1(pred1,op1+i);
        //Load a vector of op2
        VectorType svop2=svld1(pred1,op2+i);
        //(a+b)/2*N
        HalfVec svr=svaddhnt(sveven,svop1,svop2);
        //Store
        svst1(pred1,r+i,svr);

    } 
}
static void calc_vecaddhnt_ref(HalfScalar *out,HalfScalar *even,ScalarType *op1,ScalarType *op2,size_t cmputSize)
{       
    for (size_t i=1;i<cmputSize;i=i+2)//奇数 (a+b)/2*N   模运算？？？
    {
         Doublelenth temp=(Doublelenth)op1[i]+op2[i];
        if(temp>MAX_VALUE)
        {
            out[i]=MAX_RESULT;
        }
        else if(temp<MIN_VALUE)
        {
            out[i]=MIN_RESULT;
        }
        else if(temp<0)
        {
            out[i]=(((((Doublelenth)1<<sizeof(Doublelenth)*4)+temp)>>sizeof(Doublelenth)*2-1)+1)/2+(Doublelenth)2*MIN_RESULT;
        }
        else
        {
            out[i]=((temp>>sizeof(Doublelenth)*4-1)+1)/2;
        }
    }
    
    for (size_t i=0;i<cmputSize;i=i+2)//偶数 even
    {
        out[i]=even[i];
    }
}

int test_svaddhnb_uint32_vv(size_t cmputSize)
{
    HalfScalar *ref_x=(HalfScalar*)malloc(cmputSize*sizeof(HalfScalar));
    HalfScalar *opt_x=(HalfScalar*)malloc(cmputSize*sizeof(HalfScalar));
    HalfScalar *even=(HalfScalar*)malloc(cmputSize*sizeof(HalfScalar));
    ScalarType *op1=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *op2=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    int ret=0;
    srand((unsigned)time(NULL));

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((ScalarType)1<<sizeof(ScalarType)*4)+MIN_RESULT;
        opt_x[i]=bigrand()%((ScalarType)1<<sizeof(ScalarType)*4)+MIN_RESULT;
        even[i]=bigrand()%((ScalarType)1<<sizeof(ScalarType)*4)+MIN_RESULT;
        op1[i]=bigrand()%((Doublelenth)1<<sizeof(Doublelenth)*4)+MIN_VALUE;
        op2[i]=bigrand()%((Doublelenth)1<<sizeof(Doublelenth)*4)+MIN_VALUE;
    }

    calc_vecaddhnt_opt(opt_x,even,op1,op2,cmputSize);
    calc_vecaddhnt_ref(ref_x,even,op1,op2,cmputSize);
    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,even:%llu,op1:%llu,op2:%llu,ref_x=%llu,opt_x=%llu\n",i,(uint64_t)even[i],(uint64_t)op1[i],(uint64_t)op2[i],(uint64_t)ref_x[i],(uint64_t)opt_x[i]);
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
    free(even);
    even=NULL;
    free(op1);
    op1=NULL;
    free(op2);
    op2=NULL;

    return ret;
}