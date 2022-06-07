#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<math.h>
#include<time.h>
#include<arm_sve.h>

extern int test_svqdmulh_int8_vs();
extern int test_svqdmulh_int16_vs();
extern int test_svqdmulh_int32_vs();
extern int test_svqdmulh_int64_vs();

//
extern int test_svqdmulh_int8_vv();
extern int test_svqdmulh_int16_vv();
extern int test_svqdmulh_int32_vv();
extern int test_svqdmulh_int64_vv();

//
extern int test_svqdmulh_lane_int16_vv();
extern int test_svqdmulh_lane_int32_vv();
extern int test_svqdmulh_lane_int64_vv();






int main(int argc,char * argv[])
{
    int ret=0;
    srand((unsigned)time(NULL));

    size_t Size=1024*1024+2;
    printf("%s, %lu TEST ComputeSize\n",__func__,Size);
    if(argv>=2){
        Size=strtoul(argv[1],NULL,0);

    }

    printf("%s, %lu TEST ComputeSize\n",__func__,Size);

    ret+=test_svqdmulh_int8_vs(Size);
    ret+=test_svqdmulh_int16_vs(Size);
    ret+=test_svqdmulh_int32_vs(Size);
    ret+=test_svqdmulh_int64_vs(Size);

    //
    ret+=test_svqdmulh_int8_vv(Size);
    ret+=test_svqdmulh_int16_vv(Size);
    ret+=test_svqdmulh_int32_vv(Size);
    ret+=test_svqdmulh_int64_vv(Size);

    //
    ret+=test_svqdmulh_lane_int16_vv(Size);
    ret+=test_svqdmulh_lane_int32_vv(Size);
    ret+=test_svqdmulh_lane_int64_vv(Size);





    return ret;
}