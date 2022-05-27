#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

static const size_t SIZE=1024*1024+4;
static const size_t OFFSET=3;
#define ScalarType int8_t
#define VectorType svint8_t
#define WhileLE svwhilelt_b8
#define COUNT svcntb

static void calc_vecadd_opt(ScalarType c[SIZE],ScalarType a[SIZE],ScalarType b[SIZE])
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<SIZE;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLE(i,SIZE);
        svbool_t pred2=WhileLE(i,SIZE-OFFSET);
        //Load a vector of a
        VectorType sva=svld1(pred1,a+i);
        //Load a vector of b
        VectorType svb=svld1(pred1,b+i);
        //Load a vector of c
        VectorType svc=svqadd_m(pred2,sva,svb);
        //Store a+b
        svst1(pred2,c+i,svc);

    } 
}
static void calc_vecadd_ref(ScalarType out[SIZE],ScalarType a[SIZE],ScalarType b[SIZE])
{
    for (size_t i=0;i<SIZE;++i)
    {
        out[i]=a[i]+b[i];
    }
}

int test_svqadd_m_int8_vv()
{
    ScalarType *ref_x=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *opt_x=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *a=(ScalarType*)malloc(SIZE*sizeof(ScalarType));
    ScalarType *b=(ScalarType*)malloc(SIZE*sizeof(ScalarType));

    for (size_t i=0;i<SIZE;++i)
    {
        ref_x[i]=0;
        opt_x[i]=0;
        a[i]=i%(63);
        b[i]=i%(63);
    }

    calc_vecadd_opt(opt_x,a,b);
    calc_vecadd_ref(ref_x,a,b);

    for (size_t i=0;i<(SIZE-OFFSET);++i)
    {
        if(ref_x[i]!=opt_x[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,ref_x=%u,opt_x=%u\n",i,ref_x[i],opt_x[i]);

            return 1;
        }
    }

    for (size_t i=(SIZE-OFFSET);i<SIZE;++i)
    {
        if(a[i]!=opt_x[i]){
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,opt_x=%u,a=%u\n",i,opt_x[i],a[i]);
            return 1;
        }
    }
    printf("%s, %d TEST PASSED\n",__func__,__LINE__);

    return EXIT_SUCCESS;
}