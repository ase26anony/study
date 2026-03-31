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
static int helper1(int a, int b, int c) {
    return (a * b) ^ c;
}

__attribute__((noinline, noclone))
static float helper2(float a, float b, float c) {
    return (a + b) * c;
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    return a + b * 2;
}

__attribute__((noinline, noclone))
static double helper4(double a, double b, double c) {
    return a * b - c;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    int a1 = input1 + 1;
    int a2 = input1 * 2;
    int a3 = input1 ^ 0x55;
    int a4 = input1 & 0xFF;
    int a5 = input1 | 0xAA;
    int a6 = input1 << 2;
    int a7 = input1 >> 1;
    int a8 = ~input1;
    int a9 = input1 % 17;
    int a10 = input1 * 3;
    
    long b1 = input2 + 1000;
    long b2 = input2 * 3;
    long b3 = input2 / 2;
    long b4 = input2 - 500;
    long b5 = input2 ^ 0x1234;
    long b6 = input2 & 0xFFFF;
    long b7 = input2 | 0x8888;
    long b8 = input2 << 3;
    long b9 = input2 >> 2;
    long b10 = input2 % 23;
    
    float c1 = input3 + 1.5f;
    float c2 = input3 * 2.0f;
    float c3 = input3 / 3.0f;
    float c4 = input3 - 0.5f;
    float c5 = input3 * input3;
    float c6 = 1.0f / input3;
    float c7 = input3 + input3;
    float c8 = input3 * 4.0f;
    float c9 = input3 - 2.0f;
    float c10 = input3 / 0.7f;
    
    double d1 = input4 + 2.5;
    double d2 = input4 * 3.0;
    double d3 = input4 / 4.0;
    double d4 = input4 - 1.5;
    double d5 = input4 * input4;
    double d6 = 2.0 / input4;
    double d7 = input4 + input4;
    double d8 = input4 * 5.0;
    double d9 = input4 - 3.0;
    double d10 = input4 / 0.9;
    
    /* Vector variables for additional pressure */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf v3 = {c1, c2, c3, c4};
    v4sf v4 = {c5, c6, c7, c8};
    v2df v5 = {d1, d2};
    v2df v6 = {d3, d4};
    
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
    long u7 = u6 << 1;
    long u8 = u7 >> 2;
    long u9 = u8 + b8;
    long u10 = u9 * b9;
    
    float f1 = c1 + c2;
    float f2 = f1 * c3;
    float f3 = f2 - c4;
    float f4 = f3 * c5;
    float f5 = f4 / c6;
    float f6 = f5 + c7;
    float f7 = f6 * c8;
    float f8 = f7 - c9;
    float f9 = f8 / c10;
    float f10 = f9 + c1;
    
    double g1 = d1 + d2;
    double g2 = g1 * d3;
    double g3 = g2 - d4;
    double g4 = g3 * d5;
    double g5 = g4 / d6;
    double g6 = g5 + d7;
    double g7 = g6 * d8;
    double g8 = g7 - d9;
    double g9 = g8 / d10;
    double g10 = g9 + d1;
    
    /* Vector operations */
    v4si v7 = v1 + v2;
    v4si v8 = v7 * 3;
    v4si v9 = v8 - v1;
    v4si v10 = v9 ^ v2;
    
    v4sf v11 = v3 + v4;
    v4sf v12 = v11 * 2.0f;
    v4sf v13 = v12 - v3;
    v4sf v14 = v13 / 1.5f;
    
    v2df v15 = v5 + v6;
    v2df v16 = v15 * 1.5;
    v2df v17 = v16 - v5;
    v2df v18 = v17 / 2.0;
    
    /* Control flow to create multiple basic blocks */
    if (t1 > 100) {
        t2 = helper1(t1, t3, t4);
        f3 = helper2(f1, f2, f4);
    } else {
        t2 = helper1(t3, t4, t1);
        f3 = helper2(f2, f4, f1);
    }
    
    switch (u1 % 4) {
        case 0:
            v7 = helper3(v1, v2);
            g3 = helper4(g1, g2, g4);
            break;
        case 1:
            v7 = helper3(v2, v1);
            g3 = helper4(g2, g1, g4);
            break;
        case 2:
            v7 = helper3(v1, v1);
            g3 = helper4(g1, g1, g4);
            break;
        default:
            v7 = helper3(v2, v2);
            g3 = helper4(g2, g2, g4);
            break;
    }
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers\n"
        "mov $0, %%rax\n"
        "mov $0, %%rbx\n"
        "mov $0, %%rcx\n"
        "mov $0, %%rdx\n"
        "mov $0, %%rsi\n"
        "mov $0, %%rdi\n"
        "mov $0, %%r8\n"
        "mov $0, %%r9\n"
        "mov $0, %%r10\n"
        "mov $0, %%r11\n"
        "mov $0, %%r12\n"
        "mov $0, %%r13\n"
        "mov $0, %%r14\n"
        "mov $0, %%r15\n"
        "pxor %%xmm0, %%xmm0\n"
        "pxor %%xmm1, %%xmm1\n"
        "pxor %%xmm2, %%xmm2\n"
        "pxor %%xmm3, %%xmm3\n"
        "pxor %%xmm4, %%xmm4\n"
        "pxor %%xmm5, %%xmm5\n"
        "pxor %%xmm6, %%xmm6\n"
        "pxor %%xmm7, %%xmm7\n"
        "pxor %%xmm8, %%xmm8\n"
        "pxor %%xmm9, %%xmm9\n"
        "pxor %%xmm10, %%xmm10\n"
        "pxor %%xmm11, %%xmm11\n"
        "pxor %%xmm12, %%xmm12\n"
        "pxor %%xmm13, %%xmm13\n"
        "pxor %%xmm14, %%xmm14\n"
        "pxor %%xmm15, %%xmm15\n"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* ARM version (commented out, use as needed)
    asm volatile (
        "# Clobber ARM registers\n"
        "mov r0, #0\n"
        "mov r1, #0\n"
        "mov r2, #0\n"
        "mov r3, #0\n"
        "mov r4, #0\n"
        "mov r5, #0\n"
        "mov r6, #0\n"
        "mov r7, #0\n"
        "mov r8, #0\n"
        "mov r9, #0\n"
        "mov r10, #0\n"
        "vmov.i32 q0, #0\n"
        "vmov.i32 q1, #0\n"
        "vmov.i32 q2, #0\n"
        "vmov.i32 q3, #0\n"
        "vmov.i32 q4, #0\n"
        "vmov.i32 q5, #0\n"
        "vmov.i32 q6, #0\n"
        "vmov.i32 q7, #0\n"
        "vmov.i32 q8, #0\n"
        "vmov.i32 q9, #0\n"
        "vmov.i32 q10, #0\n"
        "vmov.i32 q11, #0\n"
        "vmov.i32 q12, #0\n"
        "vmov.i32 q13, #0\n"
        "vmov.i32 q14, #0\n"
        "vmov.i32 q15, #0\n"
        :
        :
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8", "r9", "r10",
          "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7",
          "q8", "q9", "q10", "q11", "q12", "q13", "q14", "q15",
          "memory"
    );
    */
    
    /* Recomputation of values in slightly different forms */
    /* This increases likelihood of early rematerialization */
    int t1_recomp = a1 + a2;  /* Same as t1 */
    int t2_recomp = t1_recomp * a3 + 1;  /* Similar to t2 but different */
    int t3_recomp = t2_recomp - a4 - 1;  /* Similar to t3 but different */
    
    long u1_recomp = b1 + b2 + 100;  /* Similar to u1 but different */
    long u2_recomp = u1_recomp * b3 / 2;  /* Similar to u2 but different */
    
    float f1_recomp = c1 + c2 + 0.1f;  /* Similar to f1 but different */
    float f2_recomp = f1_recomp * c3 * 0.5f;  /* Similar to f2 but different */
    
    double g1_recomp = d1 + d2 + 0.01;  /* Similar to g1 but different */
    double g2_recomp = g1_recomp * d3 / 1.5;  /* Similar to g2 but different */
    
    /* More complex expressions using previously computed values */
    int final1 = t10 + t2_recomp - t3_recomp;
    long final2 = u10 ^ u2_recomp | u1_recomp;
    float final3 = f10 * f2_recomp / f1_recomp;
    double final4 = g10 + g2_recomp - g1_recomp;
    
    /* Vector recomputation */
    v4si v7_recomp = v1 + v2 + 1;
    v4sf v11_recomp = v3 + v4 * 0.5f;
    v2df v15_recomp = v5 + v6 * 0.25;
    
    /* Combine everything into a final result */
    volatile int result = final1 + (int)final2 + (int)final3 + (int)final4;
    result += v7_recomp[0] + v7_recomp[1] + v7_recomp[2] + v7_recomp[3];
    result += (int)(v11_recomp[0] + v11_recomp[1] + v11_recomp[2] + v11_recomp[3]);
    result += (int)(v15_recomp[0] + v15_recomp[1]);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
    }
    
    volatile int total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int input1 = input_seed + i;
        volatile long input2 = input_seed * 3L + i * 2L;
        volatile float input3 = (float)input_seed / 3.0f + i * 0.1f;
        volatile double input4 = (double)input_seed / 7.0 + i * 0.01;
        
        int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %d\n", total);
    return total != 0 ? 0 : 1;
}
