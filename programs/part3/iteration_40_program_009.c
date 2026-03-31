/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper functions to split basic blocks */
__attribute__((noinline, noclone))
static int helper1(int a, int b) {
    return (a ^ b) + (a & b) * 2;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    if (a > b) return a * c - b;
    else return b * c + a;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, 2, 3, 4};
    return (a + b) * mask;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 ^ 0x55AA55AA;
    int a4 = input1 | 0x12345678;
    int a5 = input1 & 0xF0F0F0F0;
    int a6 = ~input1;
    int a7 = input1 << 3;
    int a8 = input1 >> 2;
    int a9 = input1 % 17;
    int a10 = input1 * input1;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 3;
    long b3 = input2 / 7;
    long b4 = input2 ^ 0xAAAAAAAAAAAAAAAALL;
    long b5 = input2 | 0x5555555555555555LL;
    long b6 = ~input2;
    long b7 = input2 << 5;
    long b8 = input2 >> 3;
    long b9 = input2 % 23;
    long b10 = input2 * input2;
    
    float c1 = input3 + 1.5f;
    float c2 = input3 * 2.5f;
    float c3 = input3 / 3.5f;
    float c4 = input3 - 4.5f;
    float c5 = -input3;
    float c6 = input3 * input3;
    float c7 = 1.0f / input3;
    float c8 = input3 + input3;
    float c9 = input3 * 0.707f;
    float c10 = input3 * 1.414f;
    
    double d1 = input4 + 2.71828;
    double d2 = input4 * 3.14159;
    double d3 = input4 / 1.61803;
    double d4 = input4 - 0.57721;
    double d5 = -input4;
    double d6 = input4 * input4;
    double d7 = 1.0 / input4;
    double d8 = input4 + input4;
    double d9 = input4 * 0.693147;
    double d10 = input4 * 1.442695;
    
    /* Vector variables */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c7, c8};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    int t2 = t1 * a3;
    int t3 = t2 - a4;
    int t4 = t3 ^ a5;
    int t5 = t4 | a6;
    int t6 = t5 & a7;
    int t7 = t6 << 2;
    int t8 = t7 >> 1;
    int t9 = t8 + a8;
    int t10 = t9 * a9;
    
    long u1 = b1 + b2;
    long u2 = u1 * b3;
    long u3 = u2 - b4;
    long u4 = u3 ^ b5;
    long u5 = u4 | b6;
    long u6 = u5 & b7;
    long u7 = u6 << 3;
    long u8 = u7 >> 2;
    long u9 = u8 + b8;
    long u10 = u9 * b9;
    
    float f1 = c1 + c2;
    float f2 = f1 * c3;
    float f3 = f2 - c4;
    float f4 = f3 * c5;
    float f5 = f4 + c6;
    float f6 = f5 / c7;
    float f7 = f6 - c8;
    float f8 = f7 * c9;
    float f9 = f8 + c10;
    float f10 = f9 * c1;
    
    double g1 = d1 + d2;
    double g2 = g1 * d3;
    double g3 = g2 - d4;
    double g4 = g3 * d5;
    double g5 = g4 + d6;
    double g6 = g5 / d7;
    double g7 = g6 - d8;
    double g8 = g7 * d9;
    double g9 = g8 + d10;
    double g10 = g9 * d1;
    
    /* Vector operations */
    v4si v3 = v1 + v2;
    v4si v4 = v3 * v1;
    v4si v5 = v4 - v2;
    v4sf vf3 = vf1 + vf2;
    v4sf vf4 = vf3 * vf1;
    v4sf vf5 = vf4 - vf2;
    v2df vd3 = vd1 + vd2;
    v2df vd4 = vd3 * vd1;
    v2df vd5 = vd4 - vd2;
    
    /* Control flow to create multiple basic blocks */
    if (t1 > 1000) {
        t2 = helper1(t1, t3);
        f3 = helper2(f1, f2, f4);
    } else {
        t2 = helper1(t3, t1);
        f3 = helper2(f2, f1, f4);
    }
    
    switch (t4 & 0x3) {
        case 0:
            v3 = helper3(v1, v2);
            break;
        case 1:
            v3 = helper3(v2, v1);
            break;
        case 2:
            v3 = v1 * v2 + v4;
            break;
        default:
            v3 = v2 * v1 - v4;
            break;
    }
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : 
        : 
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* ARM version (commented out, choose based on target)
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14",
          "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7",
          "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
          "memory"
    );
    */
    
    /* Recomputation of values in slightly different forms */
    /* This increases likelihood of early rematerialization */
    int t1_again = (a1 + a2) ^ 0x1234;  /* Similar to t1 but different */
    int t2_again = (t1_again * a3) | 0x55;  /* Similar to t2 but different */
    long u1_again = (b1 + b2) ^ 0x12345678;  /* Similar to u1 */
    long u2_again = (u1_again * b3) | 0xAA;  /* Similar to u2 */
    
    float f1_again = (c1 + c2) * 1.1f;  /* Similar to f1 */
    float f2_again = (f1_again * c3) + 0.5f;  /* Similar to f2 */
    double g1_again = (d1 + d2) * 1.01;  /* Similar to g1 */
    double g2_again = (g1_again * d3) + 0.25;  /* Similar to g2 */
    
    /* More complex operations mixing recomputed values */
    int final1 = t10 + t2_again - t1_again;
    long final2 = u10 + u2_again - u1_again;
    float final3 = f10 + f2_again - f1_again;
    double final4 = g10 + g2_again - g1_again;
    
    /* Use vector results */
    v4si v_sum = v3 + v4 + v5;
    v4sf vf_sum = vf3 + vf4 + vf5;
    v2df vd_sum = vd3 + vd4 + vd5;
    
    /* Extract elements to force vector usage */
    int v_result = v_sum[0] + v_sum[1] + v_sum[2] + v_sum[3];
    float vf_result = vf_sum[0] + vf_sum[1] + vf_sum[2] + vf_sum[3];
    double vd_result = vd_sum[0] + vd_sum[1];
    
    /* Final combination */
    volatile int result = (final1 + v_result) ^ 
                         (int)(final2 & 0xFFFFFFFF) ^
                         (int)(final3 * 1000) ^
                         (int)(final4 * 1000) ^
                         (int)(vf_result * 100) ^
                         (int)(vd_result * 100);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile int seed = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    volatile int total = 0;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        int result = test_remat(seed + i, 
                               seed2 + i * 3,
                               seed3 + i * 0.1f,
                               seed4 + i * 0.01);
        total ^= result;
        
        /* Modify seeds to prevent pattern recognition */
        seed ^= result;
        seed2 += result;
        seed3 += result * 0.001f;
        seed4 += result * 0.0001;
    }
    
    printf("Final result: %d\n", total);
    return total & 0xFF;
}
