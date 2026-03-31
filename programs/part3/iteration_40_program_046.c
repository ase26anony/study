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
    double sum = v[0] + v[1];
    if (sum > 100.0) sum *= 0.5;
    return sum;
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
    
    /* Integer temporaries */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Floating point temporaries */
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    
    /* Long temporaries */
    long l1, l2, l3, l4, l5;
    
    /* Vector variables */
    v4si vi1, vi2, vi3, vi4;
    v4sf vf1, vf2, vf3;
    v2df vd1, vd2;
    
    /* Initialize vectors */
    vi1 = (v4si){a, a+1, a+2, a+3};
    vi2 = (v4si){b%100, b%101, b%102, b%103};
    vf1 = (v4sf){c, c*2.0f, c*3.0f, c*4.0f};
    vd1 = (v2df){d, d*1.5};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (b % 100);
    t2 = t1 * (a % 50);
    t3 = t2 - (b % 75);
    t4 = t3 ^ t1;
    t5 = t4 | (t2 & 0xFF);
    t6 = t5 << 3;
    t7 = t6 >> 1;
    t8 = t7 + t3 - t1;
    t9 = t8 * 37 % 101;
    t10 = t9 ^ t6 ^ t2;
    
    f1 = c * 1.5f;
    f2 = f1 + c / 2.0f;
    f3 = f2 * f1 - c;
    f4 = f3 / (f1 + 1.0f);
    f5 = f4 * 2.0f + f2;
    
    d1 = d * 0.75;
    d2 = d1 + d / 3.0;
    d3 = d2 * d1 - d;
    d4 = d3 / (d1 + 0.5);
    d5 = d4 * 1.25 + d2;
    
    /* Vector operations */
    vi3 = vi1 + vi2;
    vi4 = vi3 * vi1 - vi2;
    vf2 = vf1 * (v4sf){f1, f2, f3, f4};
    vf3 = vf2 + vf1 / (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
    vd2 = vd1 * (v2df){d1, d2};
    
    /* Control flow to create multiple basic blocks */
    if (t10 > 1000) {
        t11 = t10 * 2;
        t12 = t11 - 500;
        f6 = f5 * 3.0f;
    } else {
        t11 = t10 / 2;
        t12 = t11 + 250;
        f6 = f5 / 2.0f;
    }
    
    /* More computations */
    t13 = t12 ^ t9;
    t14 = t13 * 17 % 97;
    t15 = t14 + t8 - t5;
    
    f7 = f6 + f4 - f2;
    f8 = f7 * f3 / f1;
    f9 = f8 - c * 0.25f;
    f10 = f9 * 2.0f;
    
    d6 = d5 + d3 - d1;
    d7 = d6 * d4 / d2;
    d8 = d7 - d * 0.125;
    d9 = d8 * 1.5;
    d10 = d9 + d6;
    
    /* Inline assembly to clobber registers */
    /* x86_64 version */
    asm volatile (
        "# Clobber many registers\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* ARM version (commented out, use for ARM targets) */
    /*
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
    
    /* Recomputation of earlier values in different forms */
    /* This increases chances for early rematerialization */
    t16 = t1 + t2;  /* Similar to earlier t8 computation */
    t17 = t16 * 3 - t3;  /* Different form of earlier pattern */
    t18 = t17 ^ t4;
    
    f7 = f1 + f2;  /* Recomputation */
    f8 = f7 * 1.5f - f3;  /* Different form */
    
    d7 = d1 + d2;  /* Recomputation */
    d8 = d7 * 0.8 + d3;  /* Different form */
    
    /* More vector operations */
    vi3 = vi1 * 2 + vi2;
    vf2 = vf1 + (v4sf){f7, f8, f9, f10};
    
    /* Switch statement for additional control flow */
    switch (t18 % 4) {
        case 0:
            t19 = t18 * 2;
            d9 = d8 * 2.0;
            break;
        case 1:
            t19 = t18 / 2;
            d9 = d8 / 2.0;
            break;
        case 2:
            t19 = t18 + 100;
            d9 = d8 + 50.0;
            break;
        default:
            t19 = t18 - 100;
            d9 = d8 - 50.0;
            break;
    }
    
    /* Call helper functions to create cross-block dataflow */
    t20 = compute_branch(t19, t15, t10);
    d10 = vector_reduce(vd2);
    
    /* Final complex expression using many temporaries */
    l1 = (long)t20 * b;
    l2 = l1 + (long)(f10 * 100.0f);
    l3 = l2 + (long)(d10 * 1000.0);
    l4 = l3 ^ (t19 * t15);
    l5 = l4 + (vi3[0] + vi3[1] + vi3[2] + vi3[3]);
    
    /* Use all major variables to ensure they're live */
    volatile long result = l5 + (long)(vf3[0] * 10.0f) + (long)vd1[0];
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int seed1 = 12345;
    volatile long seed2 = 67890;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.71828;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 6364136223846793005UL + 1) & 0x7fffffffffffffff;
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.05 + 0.25;
        
        long result = test_remat(seed1 % 1000, 
                                 seed2 % 10000,
                                 seed3,
                                 seed4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r" (total));
    }
    
    printf("Final result: %ld\n", total);
    return 0;
}
