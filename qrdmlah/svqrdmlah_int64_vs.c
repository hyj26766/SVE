#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

static const size_t SIZE=1024*1024+5;
static const size_t OFFSET=2;
#define ScalarType int64_t
#define VectorType svint64_t
#define Doublelenth __int128_t
#define WhileLT svwhilelt_b64
#define COUNT svcntd
#define Slrlen 63
#define MAX_VALUE INT64_MAX
#define MIN_VALUE INT64_MIN

static void calc_vecmlah_opt(ScalarType r[SIZE],ScalarType c[SIZE],ScalarType a[SIZE],ScalarType b)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<SIZE;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLT(i,SIZE);
        svbool_t pred2=WhileLT(i,SIZE-OFFSET);
        //Load a vector of a
        VectorType sva=svld1(pred1,a+i);
        //Load a vector of c
        VectorType svc=svld1(pred1,c+i);
        //mul svc
        VectorType svr=svqrdmlah(svc,sva,b);
        //Store c+ab
        svst1(pred2,r+i,svr);

    } 
}
static void calc_vecmlah_ref(ScalarType out[SIZE],ScalarType c[SIZE],ScalarType a[SIZE],ScalarType b)
{
    for (size_t i=0;i<SIZE;++i)
    {
        Doublelenth temp=(Doublelenth)a[i]*b;
        Doublelenth temp2=((temp>>(Slrlen-1))+1)/2 + c[i];
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

int test_svqrdmlah_int64_vs()
{
    ScalarType *ref_x=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *a=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *c=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    srand((unsigned)time(NULL));
    ScalarType b=rand()%MAX_VALUE;

    for (size_t i=0;i<SIZE;++i)
    {
        ref_x[i]=0;
        opt_x[i]=0;
        a[i]=i%((MAX_VALUE));
        c[i]=rand()%MAX_VALUE;
    }

    for (size_t i=(SIZE-OFFSET);i<SIZE;++i)
    {
        opt_x[i]=100;
    }

    calc_vecmlah_opt(opt_x,c,a,b);
    calc_vecmlah_ref(ref_x,c,a,b);

    for (size_t i=0;i<(SIZE-OFFSET);++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,ref_x=%lld,opt_x=%lld\n",i,ref_x[i],opt_x[i]);

            return 1;
        }
    }

    for (size_t i=(SIZE-OFFSET);i<SIZE;++i)
    {
        if(100!=opt_x[i]){
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,opt_x=%lld\n",i,opt_x[i]);
            return 1;
        }
    }
    printf("%s, %d TEST PASSED\n",__func__,__LINE__);

    return EXIT_SUCCESS;
}

int main()
{
    test_svqrdmlah_int64_vs();
    return 1;
}