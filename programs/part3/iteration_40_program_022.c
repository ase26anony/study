/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper to split basic blocks */
__attribute__((noinline, noclone))
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c - b;
    } else if (a < b) {
        return b * c + a;
    } else {
        return (a + b) * c;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v4sf v) {
    double sum = v[0] + v[1] + v[2] + v[3];
    return sum * 0.25;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2,
                                  volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 distinct ones */
    volatile int a = input1 + 1;
    volatile long b = input2 - 2;
    volatile float c = input3 * 3.14f;
    volatile double d = input4 / 2.71828;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5, f6;
    double d1, d2, d3, d4, d5, d6, d7;
    
    /* Vector variables */
    v4si vint1 = {a, a+1, a+2, a+3};
    v4si vint2 = {b%10, b%20, b%30, b%40};
    v4sf vfloat1 = {c, c*2, c*3, c*4};
    v4sf vfloat2 = {d, d*0.5, d*0.25, d*0.125};
    v2df vdouble1 = {d, d*1.5};
    v2df vdouble2 = {d*2.0, d*2.5};
    
    /* Long serial chain of interdependent operations */
    t1 = a * 3 + (b % 100);
    t2 = t1 - (a / 2);
    t3 = t2 * compute_branch(a, t1, t2);
    t4 = t3 ^ (t1 << 2);
    t5 = (t4 > t3) ? t4 : t3;
    t6 = t5 + compute_branch(t2, t3, t4);
    
    l1 = b * t1;
    l2 = l1 + t2 * 17L;
    l3 = l2 - (t3 << 3);
    l4 = l3 ^ (l1 >> 2);
    l5 = (l4 > l2) ? l4 : l2;
    
    f1 = c + t1 * 0.5f;
    f2 = f1 * c - t2 * 0.25f;
    f3 = f2 / (c + 1.0f);
    f4 = f3 + compute_branch(t1, t2, t3) * 0.1f;
    f5 = f4 * f2 - f3;
    f6 = f5 / (f1 + 0.001f);
    
    d1 = d + l1 * 0.01;
    d2 = d1 * d - f1 * 0.02;
    d3 = d2 / (d + 0.5);
    d4 = d3 + compute_branch(t4, t5, t6) * 0.001;
    d5 = d4 * d2 - d3;
    d6 = d5 / (d1 + 0.0001);
    d7 = d6 + vector_reduce(vfloat1);
    
    /* Vector operations mixed with scalar */
    vint1 = vint1 + vint2 * 2;
    vint2 = vint1 - vint2;
    vfloat1 = vfloat1 * vfloat2 + (v4sf){f1, f2, f3, f4};
    vfloat2 = vfloat2 / vfloat1 - (v4sf){0.1f, 0.2f, 0.3f, 0.4f};
    vdouble1 = vdouble1 + vdouble2 * 0.3;
    vdouble2 = vdouble2 - vdouble1 * 0.7;
    
    /* Inline assembly to clobber physical registers (x86_64 version) */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "mov $0, %%rax\n\t"
        "mov $0, %%rbx\n\t"
        "mov $0, %%rcx\n\t"
        "mov $0, %%rdx\n\t"
        "mov $0, %%rsi\n\t"
        "mov $0, %%rdi\n\t"
        "mov $0, %%r8\n\t"
        "mov $0, %%r9\n\t"
        "mov $0, %%r10\n\t"
        "mov $0, %%r11\n\t"
        "mov $0, %%r12\n\t"
        "mov $0, %%r13\n\t"
        "mov $0, %%r14\n\t"
        "mov $0, %%r15\n\t"
        "pxor %%xmm0, %%xmm0\n\t"
        "pxor %%xmm1, %%xmm1\n\t"
        "pxor %%xmm2, %%xmm2\n\t"
        "pxor %%xmm3, %%xmm3\n\t"
        "pxor %%xmm4, %%xmm4\n\t"
        "pxor %%xmm5, %%xmm5\n\t"
        "pxor %%xmm6, %%xmm6\n\t"
        "pxor %%xmm7, %%xmm7\n\t"
        "pxor %%xmm8, %%xmm8\n\t"
        "pxor %%xmm9, %%xmm9\n\t"
        "pxor %%xmm10, %%xmm10\n\t"
        "pxor %%xmm11, %%xmm11\n\t"
        "pxor %%xmm12, %%xmm12\n\t"
        "pxor %%xmm13, %%xmm13\n\t"
        "pxor %%xmm14, %%xmm14\n\t"
        "pxor %%xmm15, %%xmm15"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory", "cc"
    );
    
    /* More operations after clobber - forcing recomputation */
    t7 = t6 * 2 + (a % 7);  /* Recompute similar to t6 pattern */
    t8 = t7 - (t5 >> 1);
    t9 = t8 ^ compute_branch(t6, t7, t8);
    t10 = (t9 < t8) ? t9 : t8;
    
    /* Complex expression computed, used, then recomputed */
    int complex_val = (t3 * t4 + t5) / (t6 + 1);
    d7 += complex_val * 0.01;
    
    /* Use complex_val in multiple places */
    f6 += complex_val * 0.1f;
    l5 += complex_val * 100L;
    
    /* Recompute similar complex expression later */
    int complex_val2 = (t7 * t8 + t9) / (t10 + 1);  /* Similar pattern */
    d7 -= complex_val2 * 0.005;
    
    /* Control flow to split basic blocks */
    if (d7 > 1000.0) {
        d7 = d7 * 0.9;
        vfloat1 = vfloat1 * 0.5f;
    } else if (d7 < -1000.0) {
        d7 = d7 * 1.1;
        vfloat2 = vfloat2 * 1.5f;
    } else {
        d7 = d7 + 50.0;
        vint1 = vint1 + 1;
    }
    
    switch (t10 % 5) {
        case 0:
            d7 += vfloat1[0] + vfloat2[1];
            break;
        case 1:
            d7 += vfloat1[1] + vfloat2[2];
            break;
        case 2:
            d7 += vfloat1[2] + vfloat2[3];
            break;
        case 3:
            d7 += vfloat1[3] + vfloat2[0];
            break;
        default:
            d7 += vector_reduce(vfloat1);
            break;
    }
    
    /* Final combination to ensure all values are live */
    volatile double result = d7 + f6 + l5 + t10 + 
                           vint1[0] + vint1[1] + vint1[2] + vint1[3] +
                           vdouble1[0] + vdouble1[1];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789L;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005L + 1442695040888963407L;
        seed3 = seed3 * 1.01f + 0.5f;
        seed4 = seed4 * 1.001 + 0.1;
        
        total += test_remat(seed1 % 1000, 
                           seed2 % 10000, 
                           seed3, 
                           seed4);
    }
    
    printf("Result: %f\n", (double)total);
    return 0;
}
