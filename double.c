#include <iostream>
#include <algorithm>
#define U64 uint64_t
#define U32 uint32_t
 
#define GREEN        "\033[0;32;32m"
#define RED          "\033[0;32;31m"
#define NONE         "\033[0m"
 
//64左移len 位
U64 move_left64(U64 a, int len) {
    return a * (0x1 << len);
}
//64右移len 位
U64 move_right64(U64 a, int len) {
    return a / (0x1 << len);
}
 
float test(double d_a) {
    uint32_t dst = 0x000000;
    float f_a = (float)d_a;
 
    uint64_t sign_d_a = (*((int64_t *)&d_a) & (0x8000000000000000)) >> 63;
    uint32_t sign_f_a = sign_d_a << 31;
    dst += sign_f_a;
 
    uint64_t exp_d_a = (*((int64_t *)&d_a) & (0x7ff0000000000000)) >> 52;
 
    uint32_t exp_f_a = std::max(std::min((int32_t)exp_d_a - 1023 + 127, 0xff), 0x0) << 23;
    int32_t denormal_shift = std::min(std::max(-((int32_t)exp_d_a - 1023 + 127) + 1, 0x0), 23);
    dst += exp_f_a;
 
    int32_t tail_mask = (exp_d_a > 1151 || exp_d_a < 874) ? 0x0 : 0x007fffff;
 
    uint32_t denormal_bit = move_right64(0x800000, denormal_shift) & tail_mask;
    dst += denormal_bit;
    
    uint64_t tail_d_a = (*((int64_t *)&d_a) & (0x000fffffffffffff));
    uint32_t tail_f_a = (tail_d_a >> 29 >> denormal_shift) & tail_mask;
    dst += tail_f_a;
 
    int64_t rn_mask = (move_left64(0x0000000020000000, denormal_shift) - 1);
    int64_t rn_base = (move_left64(0x0000000010000000, denormal_shift));
 
    uint64_t rn = tail_d_a & rn_mask;
 
    uint32_t rn_up_judge1 = ((rn > rn_base) ? 0x1 : 0x0) & tail_mask;
    dst += rn_up_judge1;
    uint64_t last_bit = 0x00000001 & dst;
    uint32_t rn_up_judge2 = ((rn == rn_base && last_bit) ? 0x1 : 0x0) & tail_mask;
    dst += rn_up_judge2;
 
    uint32_t nan_mask = (exp_d_a == 0x7ff && tail_d_a != 0) ? 0x400000 : 0x0;
    dst += nan_mask;
 
    float reference = (float)d_a;
    if (*((int32_t *)&reference) == dst) {
        printf("dst: %x, %f,  line: %d\n", dst, *((float *)&dst), __LINE__);
        printf("ref: %x, %f,  line: %d\n", *((int32_t *)&reference), reference, __LINE__);
        printf(GREEN "pass!" NONE "\n");
    } else {
        float reference = (float)d_a;
 
        printf("sign d: %lld\t %llx\t\t\t line: %d\n", sign_d_a, sign_d_a, __LINE__);
        printf("sign f: %d\t %x\t\t\t line: %d\n", sign_f_a, sign_f_a, __LINE__);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
 
        printf("*******\n");
        printf("exp d: %lld\t %llx\t\t\t line: %d\n", exp_d_a, exp_d_a, __LINE__);
 
        printf("exp f: %d\t %x\t\t\t line: %d\n", exp_f_a, exp_f_a, __LINE__);
        printf("denormal shift:\t %d, line: %d\n", denormal_shift, __LINE__);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
 
        printf("*******\n");
        printf("tail_mask:\t %x, line: %d\n", tail_mask, __LINE__);
 
        printf("denormal bit:\t %x\n", denormal_bit);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
 
        printf("*******\n");
        printf("tail f a: %x\t\t\t\t line: %d\n", tail_f_a, __LINE__);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
 
        printf("rn_up_judge1: %x\t\t\t\t line: %d\n", rn_up_judge1, __LINE__);
        printf("rn_up_judge2: %x\t\t\t\t line: %d\n", rn_up_judge2, __LINE__);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
        printf("nan mask: %d\t %x\t\t\t line: %d\n", nan_mask, nan_mask, __LINE__);
        printf("dst  f: %2.3f\t %x\t\t\t line: %d\n", *((float *)&dst), dst, __LINE__);
 
        printf("dst: %x, %f,  line: %d\n", dst, *((float *)&dst), __LINE__);
        printf("ref: %x, %f,  line: %d\n", *((int32_t *)&reference), reference, __LINE__);
        printf(RED "wrong!\n" NONE);
    }
    return *((float *)&dst);
}
 
int main() {
  int64_t from = 0x7Ff0000000000006;
  double test_value = *((double *)&from);
  test(test_value);
 
  from = 0x36a7000000000000;
  test_value = *((double *)&from);
  test(test_value);
 
  from = 0x36a8000000000000;
  test_value = *((double *)&from);
  test(test_value);
}