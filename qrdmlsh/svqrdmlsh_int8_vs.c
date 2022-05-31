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

static void calc_vecmlsh_opt(ScalarType *r,ScalarType *c,ScalarType *a,ScalarType b,size_t cmputSize)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<cmputSize;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,cmputSize);
        //Load a vector of a
        VectorType sva=svld1(pred1,a+i);
        //Load a vector of c
        VectorType svc=svld1(pred1,c+i);
        //mul svc
        VectorType svr=svqrdmlsh(svc,sva,b);
        //Store c-ab
        svst1(pred1,r+i,svr);

    } 
}
static void calc_vecmlsh_ref(ScalarType *out,ScalarType *c,ScalarType *a,ScalarType b,size_t cmputSize)
{
    for (size_t i=0;i<cmputSize;++i)
    {
        Doublelenth temp=(Doublelenth)a[i]*(Doublelenth)b;
        Doublelenth temp2;
        if(temp>0)//四舍五入
        {
            temp2=(Doublelenth)c[i]-((temp>>Slrlen-1)+1)/2;
            }
        else{
            temp2=(Doublelenth)c[i]-((((Doublelenth)2<<2*Slrlen-1)+temp)>>Slrlen-1)/2-MIN_VALUE;
            }
        if(temp2>MAX_VALUE)
        {
            out[i]=MAX_VALUE;
        }
        else if(temp2<MIN_VALUE)
        {
            out[i]=MIN_VALUE;
        }
        else{
            out[i]=temp2;
        }
    }
}

int test_svqrdmlsh_int8_vs(size_t cmputSize)
{
    ScalarType *ref_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *a=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));
    ScalarType *c=(ScalarType*)malloc(cmputSize*sizeof(ScalarType));

    int ret=0;
    srand((unsigned)time(NULL));
    ScalarType b=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;

    for (size_t i=0;i<cmputSize;++i)
    {
        ref_x[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
        opt_x[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
        a[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
        c[i]=bigrand()%((Doublelenth)2<<Slrlen)+MIN_VALUE;
    }

    calc_vecmlsh_opt(opt_x,c,a,b,cmputSize);
    calc_vecmlsh_ref(ref_x,c,a,b,cmputSize);

    for (size_t i=0;i<cmputSize;++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,c:%lld,a:%lld,b:%lld,ref_x=%lld,opt_x=%lld\n",i,(int64_t)c[i],(int64_t)a[i],(int64_t)b,(int64_t)ref_x[i],(int64_t)opt_x[i]);
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
    free(c);
    a=NULL;

    return ret;
}