/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Vector types for register pressure */
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));

/* Helper function to split basic blocks */
__attribute__((noinline, noclone))
static int compute_branch(int a, int b, int c) {
    if (a > b) {
        return a * c + b;
    } else if (a < b) {
        return b * c - a;
    } else {
        return (a + b) * c;
    }
}

__attribute__((noinline, noclone))
static double vector_compute(v4sf vec1, v4sf vec2) {
    v4sf result = vec1 + vec2 * 2.5f;
    float sum = result[0] + result[1] + result[2] + result[3];
    return (double)sum;
}

/* Main test function with high register pressure */
__attribute__((noinline))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* More scalar variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si vint1, vint2, vint3, vresult;
    v4sf vfloat1, vfloat2, vfloat3;
    v2df vdouble1, vdouble2;
    
    /* Initialize vectors */
    vint1 = (v4si){a, a+1, a+2, a+3};
    vint2 = (v4si){b%100, b%101, b%102, b%103};
    vfloat1 = (v4sf){c, c*1.1f, c*1.2f, c*1.3f};
    vfloat2 = (v4sf){d, (float)d/2, (float)d/3, (float)d/4};
    vdouble1 = (v2df){d, d*1.5};
    vdouble2 = (v2df){input4/2, input4/3};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)(c * 10);
    t3 = t2 - (int)(d * 100);
    t4 = t3 / (a + 1);
    t5 = t4 | (t2 & 0xFF);
    t6 = t5 ^ t3;
    t7 = t6 << 2;
    t8 = t7 >> 1;
    t9 = t8 + t1 - t4;
    t10 = t9 * t2 / (t3 + 1);
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - (long)(d * 1000);
    l4 = l3 / (b + 1);
    l5 = l4 | (l2 & 0xFFFF);
    l6 = l5 ^ l3;
    l7 = l6 << 3;
    l8 = l7 >> 2;
    l9 = l8 + l1 - l4;
    l10 = l9 * l2 / (l3 + 1);
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - (float)(d * 100);
    f4 = f3 / (c + 1.0f);
    f5 = f4 * 1.5f + f1;
    f6 = f5 - f2 * 0.5f;
    f7 = f6 / (f3 + 1.0f);
    f8 = f7 * 2.0f - f4;
    f9 = f8 + f1 * f5;
    f10 = f9 / (f6 + 0.1f) * f2;
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - (d * 10000.0);
    d4 = d3 / (d + 1.0);
    d5 = d4 * 1.7 + d1;
    d6 = d5 - d2 * 0.3;
    d7 = d6 / (d3 + 1.0);
    d8 = d7 * 3.0 - d4;
    d9 = d8 + d1 * d5;
    d10 = d9 / (d6 + 0.01) * d2;
    
    /* Vector operations */
    vint3 = vint1 + vint2 * 2;
    vresult = vint3 - vint1;
    vfloat3 = vfloat1 + vfloat2 * 1.5f;
    
    /* Inline assembly to clobber registers */
    /* For x86_64 */
    asm volatile (
        "# Clobber many registers\n\t"
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
          "memory"
    );
    
    /* Control flow to split basic blocks */
    if (t10 > 1000) {
        t1 = compute_branch(t10, t9, t8);
        vresult = vresult + (v4si){t1, t1/2, t1/3, t1/4};
    } else if (t10 < -1000) {
        t2 = compute_branch(-t10, t8, t7);
        vresult = vresult - (v4si){t2, t2/2, t2/3, t2/4};
    } else {
        t3 = compute_branch(t10, t7, t6);
        vresult = vresult * (v4si){t3%10, t3%9, t3%8, t3%7};
    }
    
    /* Recompute complex expression in slightly different form */
    /* This increases chances for early rematerialization */
    int recomputed_t1 = a + (int)b + (t10 % 100);
    long recomputed_l1 = b + t1 + (l10 % 1000);
    float recomputed_f1 = c + t1 + (f10 * 0.01f);
    double recomputed_d1 = d + l1 + (d10 * 0.001);
    
    /* Use the recomputed values */
    t4 = recomputed_t1 * 2 - t9;
    l4 = recomputed_l1 * 3 - l9;
    f4 = recomputed_f1 * 1.5f - f9;
    d4 = recomputed_d1 * 2.5 - d9;
    
    /* More vector operations */
    vfloat3 = vfloat3 + (v4sf){recomputed_f1, f4, f5, f6};
    vdouble2 = vdouble2 + (v2df){recomputed_d1, d4};
    
    /* Switch statement for additional control flow */
    switch (t10 % 5) {
        case 0:
            vresult = vresult + vint1;
            break;
        case 1:
            vresult = vresult - vint2;
            break;
        case 2:
            vresult = vresult * (v4si){2, 2, 2, 2};
            break;
        case 3:
            vresult = vresult / (v4si){2, 2, 2, 2};
            break;
        default:
            vresult = vresult & vint3;
            break;
    }
    
    /* Final computation using all temporaries */
    volatile long final_result = 
        (long)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
        l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
        (long)f1 + (long)f2 + (long)f3 + (long)f4 + (long)f5 + 
        (long)f6 + (long)f7 + (long)f8 + (long)f9 + (long)f10 +
        (long)d1 + (long)d2 + (long)d3 + (long)d4 + (long)d5 +
        (long)d6 + (long)d7 + (long)d8 + (long)d9 + (long)d10 +
        vresult[0] + vresult[1] + vresult[2] + vresult[3] +
        (long)vfloat3[0] + (long)vfloat3[1] + (long)vfloat3[2] + (long)vfloat3[3] +
        (long)vdouble2[0] + (long)vdouble2[1];
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile long result = test_remat(
            input1 + (i % 10),
            input2 + i,
            input3 + (i * 0.01f),
            input4 + (i * 0.001)
        );
        total += result;
        
        /* Additional computation to prevent loop optimization */
        if (i % 100 == 0) {
            input1 = (input1 * 13 + 17) % 100;
            input2 = (input2 * 17 + 13) % 1000000;
        }
    }
    
    printf("Final result: %ld\n", total);
    return 0;
}
