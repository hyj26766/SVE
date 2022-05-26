#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

static const size_t SIZE=1024*1024+5;
static const size_t OFFSET=2;
#define ScalarType int32_t
#define ScalarType1 uint32_t
#define VectorType svint32_t
#define VectorType1 svuint32_t
#define WhileLE svwhilelt_b32
#define MAX_VALUE UNIT_MAX
#define MIN_VALUE 0
#define COUNT svcntw


static void calc_vecadd_opt(ScalarType1 c1[SIZE],ScalarType1 c2[SIZE],ScalarType1 c3[SIZE],ScalarType1 c3[SIZE],ScalarType b)
{
    //Stride by the number of words in the vector
    for (size_t i=0;i<SIZE;i+=COUNT())
    {
        //Operate on vector lanes where i<SIZE
        svbool_t pred1=WhileLE(i,SIZE);
        svbool_t pred2=WhileLE(i,SIZE-OFFSET);
        //Load a vector of a
        VectorType1 sva=svld1(pred1,a+i);
        //Add a to b
        VectorType1 svc1=svqadd_z(pred2,sva,b);
        VectorType1 svc2=svqadd_x(pred2,sva,b);
        VectorType1 svc3=svqadd_m(pred2,sva,b);
        //Store a+b
        svst1(pred1,c1+i,svc1);
        svst1(pred2,c2+i,svc2);
        svst1(pred1,c3+i,svc3);
    } 
}
static void calc_vecadd_ref(ScalarType1 out[SIZE],ScalarType1 a[SIZE],ScalarType b)
{
    for (size_t i=0;i<SIZE;++i)
    {
        if((a[i]+b)>MAX_VALUE)
        {
            out[i]=MAX_VALUE;
        }
        else if((a[i]+b)<MIN_VALUE)
        {
            out[i]=MIN_VALUE;
        }
        else{
            a[i]+b;
        }
        
    }
}

int test_svsqadd_zxm_uint32_int32_vs()
{
    srand((unsigned)time(NULL));
    ScalarType1 *ref_x=(ScalarType1*)malloc(SIZE*sizeof(ScalarType1));
    ScalarType1 *opt_x1=(ScalarType1*)malloc(SIZE*sizeof(ScalarType1));
    ScalarType1 *opt_x2=(ScalarType1*)malloc(SIZE*sizeof(ScalarType1));
    ScalarType1 *opt_x3=(ScalarType1*)malloc(SIZE*sizeof(ScalarType1));
    ScalarType1 *a=(ScalarType1*)malloc(SIZE*sizeof(ScalarType1));
    ScalarType b=rand()%MAX_VALUE;

    for (size_t i=0;i<SIZE;++i)
    {
        ref_x[i]=0;
        opt_x1[i]=0;
        opt_x2[i]=0;
        opt_x3[i]=0;
        a[i]=i%(MAX_VALUE);
    }

    for (size_t i=(SIZE-OFFSET);i<SIZE;++i)
    {
        opt_x2[i]=100;
    }

    calc_vecadd_opt(opt_x1,opt_x2,opt_x3,a,b);
    calc_vecadd_ref(ref_x,a,b);

    for (size_t i=0;i<(SIZE-OFFSET);++i)
    {
        if(ref_x[i]!=opt_x1[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,ref_x=%u,opt1_x=%u\n",i,ref_x[i],opt_x1[i]);

            return 1;
        }
        if(ref_x[i]!=opt_x2[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,ref_x=%u,opt2_x=%u\n",i,ref_x[i],opt_x2[i]);

            return 1;
        }
        if(ref_x[i]!=opt_x3[i])
        {
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,ref_x=%u,opt3_x=%u\n",i,ref_x[i],opt_x3[i]);

            return 1;
        }
    }

    for (size_t i=(SIZE-OFFSET);i<SIZE;++i)
    {
        if(0!=opt_x1[i]){
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,opt1_x=%u\n",i,opt_x1[i]);
            return 1;
        }
        if(100!=opt_x2[i]){
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,opt2_x=%u\n",i,opt_x2[i]);
            return 1;
        }
        if(a[i]!=opt_x3[i]){
            printf("%s, %d TEST FAILED\n",__func__,__LINE__);
            printf("ERROR:%lu,opt_x3=%u\n",i,opt_x3[i]);
            return 1;
        }
    }
    printf("%s, %d TEST PASSED\n",__func__,__LINE__);

    return EXIT_SUCCESS;
}