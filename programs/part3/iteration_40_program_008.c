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
static int compute_complex(int a, int b, int c) {
    if (a > b) {
        return (a * c) + (b << 3);
    } else {
        return (b * c) - (a >> 2);
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    double sum = v[0] + v[1];
    if (sum > 1000.0) {
        return sum * 0.5;
    } else {
        return sum * 2.0;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2, 
                                  volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a = input1;
    volatile long b = input2;
    volatile float c = input3;
    volatile double d = input4;
    
    /* More scalar variables */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5, f6;
    double d1, d2, d3, d4, d5, d6, d7;
    
    /* Vector variables */
    v4si vint1, vint2, vint3;
    v4sf vfloat1, vfloat2;
    v2df vdouble1, vdouble2;
    
    /* Initialize vectors */
    vint1 = (v4si){a, a+1, a+2, a+3};
    vint2 = (v4si){b%100, b%101, b%102, b%103};
    vfloat1 = (v4sf){c, c*2.0f, c*3.0f, c*4.0f};
    vdouble1 = (v2df){d, d*1.5};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (a % 17);
    t3 = t2 - (int)(b >> 4);
    t4 = t3 ^ (t1 << 2);
    t5 = compute_complex(t4, t2, t3);
    
    l1 = b * 37L;
    l2 = l1 + (long)t5 * 19L;
    l3 = l2 - (l1 >> 3);
    l4 = l3 ^ (b << 5);
    l5 = l4 % 9973;
    
    f1 = c * 1.5f;
    f2 = f1 + (float)t1 * 0.25f;
    f3 = f2 - (float)l2 * 0.001f;
    f4 = f3 * f2;
    f5 = f4 / (f1 + 1.0f);
    f6 = f5 * 2.0f - f3;
    
    d1 = d * 2.71828;
    d2 = d1 + (double)f6;
    d3 = d2 * (double)l3 * 0.0001;
    d4 = d3 - d1 * 0.5;
    d5 = d4 / (d2 + 0.001);
    d6 = d5 * 3.14159;
    d7 = d6 + d3 - d4;
    
    /* Vector operations */
    vint3 = vint1 + vint2;
    vint3 = vint3 * (v4si){2, 3, 4, 5};
    vint3 = vint3 - vint1;
    
    vfloat2 = vfloat1 * (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    vfloat2 = vfloat2 + vfloat1;
    
    vdouble2 = vdouble1 * (v2df){0.333, 0.667};
    vdouble2 = vdouble2 + vdouble1;
    
    /* Inline assembly to clobber registers (x86_64 version) */
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
          "memory"
    );
    
    /* More operations after clobber - forcing recomputation */
    t6 = t5 + (t4 >> 1);  /* Reuse t4, t5 which might need rematerialization */
    t7 = t6 * (t3 % 31);  /* Reuse t3 */
    t8 = compute_complex(t7, t6, t5);
    t9 = t8 ^ (t2 << 3);  /* Reuse t2 */
    t10 = t9 - (t1 & 0xFF);  /* Reuse t1 */
    
    d7 = d7 + vector_reduce(vdouble2);  /* Use vector result */
    d7 = d7 + (double)(vint3[0] + vint3[1] + vint3[2] + vint3[3]);
    d7 = d7 + (double)(vfloat2[0] + vfloat2[1] + vfloat2[2] + vfloat2[3]);
    
    /* Control flow to split basic blocks */
    if (d7 > 10000.0) {
        d7 = d7 * 0.9;
        t10 = t10 * 2;
    } else if (d7 < -10000.0) {
        d7 = d7 * 1.1;
        t10 = t10 / 2;
    } else {
        d7 = d7 + 500.0;
        t10 = t10 + 100;
    }
    
    switch (t10 % 5) {
        case 0: d7 = d7 * 1.01; break;
        case 1: d7 = d7 * 0.99; break;
        case 2: d7 = d7 + 10.0; break;
        case 3: d7 = d7 - 10.0; break;
        default: d7 = d7 * 0.95; break;
    }
    
    /* Final computation using all major temporaries */
    volatile double result = d7 
                           + (double)t10 * 0.001 
                           + (double)l5 * 0.00001 
                           + (double)f6;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count < 1) loop_count = 1000;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 12345;
    volatile long seed2 = 67890L;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005L + 1442695040888963407L;
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.05 + 0.3;
        
        double result = test_remat(seed1 % 1000, 
                                   seed2 % 10000, 
                                   seed3, 
                                   seed4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %f\n", (double)total);
    return 0;
}
