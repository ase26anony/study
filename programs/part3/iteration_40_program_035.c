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
    } else {
        return b * c + a;
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    return v[0] + v[1];
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input1, volatile long input2, 
                               volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Vector variables */
    v4si vint1, vint2, vint3, vint4;
    v4sf vfloat1, vfloat2, vfloat3;
    v2df vdouble1, vdouble2;
    
    /* Initialize vectors */
    vint1 = (v4si){a, a+1, a+2, a+3};
    vint2 = (v4si){b%100, b%101, b%102, b%103};
    vfloat1 = (v4sf){c, c*2, c*3, c*4};
    vdouble1 = (v2df){d, d*1.5};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (b % 100);
    t2 = t1 * (a % 50);
    t3 = t2 - (b % 75);
    t4 = t3 ^ t1;
    t5 = (t4 << 3) | t2;
    t6 = t5 * t3 - t4;
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - t3;
    l4 = l3 ^ l1;
    l5 = (l4 << 2) | l2;
    l6 = l5 * l3 - l4;
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - t3;
    f4 = f3 / (f1 + 1.0f);
    f5 = f4 * f2 - f3;
    f6 = f5 / (f4 + 1.0f);
    
    d1 = d + l1;
    d2 = d1 * l2;
    d3 = d2 - l3;
    d4 = d3 / (d1 + 1.0);
    d5 = d4 * d2 - d3;
    d6 = d5 / (d4 + 1.0);
    
    /* Vector operations */
    vint3 = vint1 + vint2;
    vint4 = vint3 * vint1 - vint2;
    
    vfloat2 = vfloat1 * 2.5f;
    vfloat3 = vfloat2 + vfloat1 / 3.0f;
    
    vdouble2 = vdouble1 * 1.7;
    
    /* Control flow to create basic blocks */
    if (t1 > 1000) {
        t7 = t6 * 2;
        l7 = l6 / 3;
    } else {
        t7 = t6 / 2;
        l7 = l6 * 3;
    }
    
    /* Switch for more basic blocks */
    switch (t2 % 4) {
        case 0:
            t8 = t7 + 100;
            break;
        case 1:
            t8 = t7 - 100;
            break;
        case 2:
            t8 = t7 * 2;
            break;
        default:
            t8 = t7 / 2;
            break;
    }
    
    /* Inline assembly to clobber registers */
    /* x86_64 version - adjust for your target */
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
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "memory"
    );
    
    /* Recomputation of earlier values in different forms */
    /* This increases chances for early rematerialization */
    t9 = t1 + t2;  /* Similar to earlier t1, t2 computation */
    t10 = t9 * t3 - t4;  /* Similar to t6 computation */
    
    l8 = l1 + l2;
    l9 = l8 * l3 - l4;
    
    f7 = f1 + f2;
    f8 = f7 * f3 - f4;
    
    d7 = d1 + d2;
    d8 = d7 * d3 - d4;
    
    /* More complex expression using helper function */
    t10 = compute_branch(t9, t10, a);
    
    /* Vector reduction */
    d9 = vector_reduce(vdouble2);
    
    /* Final computation using all temporaries */
    volatile long result = (long)t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                          l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 +
                          (long)f1 + (long)f2 + (long)f3 + (long)f4 + 
                          (long)f5 + (long)f6 + (long)f7 + (long)f8 +
                          (long)d1 + (long)d2 + (long)d3 + (long)d4 +
                          (long)d5 + (long)d6 + (long)d7 + (long)d8 + (long)d9 +
                          vint3[0] + vint4[1] + (long)vfloat3[2];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile long total = 0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005ULL + 1442695040888963407ULL;
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.05 + 0.25;
        
        total += test_remat(seed1 % 1000, 
                           seed2 % 1000000, 
                           seed3, 
                           seed4);
    }
    
    printf("Result: %ld\n", total);
    return 0;
}
