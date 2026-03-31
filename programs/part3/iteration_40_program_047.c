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
static int compute_magic(int a, int b, int c) {
    if (a > b) {
        return (a * c) | (b << 3);
    } else {
        return (b * a) ^ (c & 0xFF);
    }
}

__attribute__((noinline, noclone))
static double vector_reduce(v2df v) {
    double sum = v[0] + v[1];
    if (sum > 100.0) {
        return sum * 0.5;
    } else {
        return sum * 2.0;
    }
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
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d1, d2, d3, d4, d5, d6, d7, d8;
    
    /* Long temporaries */
    long l1, l2, l3, l4, l5;
    
    /* Vector variables */
    v4si vec1, vec2, vec3, vec4;
    v4sf fvec1, fvec2;
    v2df dvec1, dvec2;
    
    /* Initialize vectors */
    vec1 = (v4si){a, a+1, a+2, a+3};
    vec2 = (v4si){b&0xFF, (b>>8)&0xFF, (b>>16)&0xFF, (b>>24)&0xFF};
    fvec1 = (v4sf){c, c*2.0f, c*3.0f, c*4.0f};
    dvec1 = (v2df){d, d*1.5};
    
    /* Start long serial chain of interdependent operations */
    t1 = a + (b & 0xFFFF);
    t2 = t1 * (a | 0x7F);
    t3 = t2 - (b >> 16);
    t4 = t3 ^ (t1 & t2);
    t5 = (t4 << 3) | (t3 >> 2);
    
    /* Mix in floating point */
    f1 = c * 1.1f;
    f2 = f1 + (float)t5;
    f3 = f2 * 0.9f - c;
    
    /* Vector operations */
    vec3 = vec1 + vec2;
    vec4 = vec3 * (v4si){t1, t2, t3, t4};
    
    /* Control flow to create basic blocks */
    if (t5 > 1000) {
        t6 = compute_magic(t1, t2, t3);
        f4 = f3 * 2.0f;
    } else {
        t6 = compute_magic(t4, t5, t1);
        f4 = f3 * 0.5f;
    }
    
    d1 = (double)f4 + d;
    d2 = d1 * 1.23456789;
    
    /* More integer chain */
    t7 = t6 & 0xFF00FF;
    t8 = t7 + t5;
    t9 = t8 * 3;
    t10 = t9 - t6;
    t11 = (t10 << 1) | (t9 >> 1);
    
    /* Inline assembly to clobber registers (x86_64 version) */
    asm volatile(
        "# Clobber many registers to increase pressure\n\t"
        "nop"
        : /* no outputs */
        : /* no inputs */
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Continue computation after clobber */
    l1 = (long)t11 * b;
    l2 = l1 + (b << 3);
    l3 = l2 ^ (l1 >> 4);
    
    /* More floating point */
    f5 = f4 * 1.5f;
    f6 = f5 - c;
    d3 = d2 + (double)f6;
    
    /* Another vector operation */
    fvec2 = fvec1 * (v4sf){f1, f2, f3, f4};
    dvec2 = dvec1 + (v2df){d2, d3};
    
    /* Complex expression that might be rematerialized */
    int complex_expr = ((t11 * 3) + (t10 >> 2)) ^ (t9 & 0xF0F0);
    
    /* Use complex_expr multiple times */
    t12 = complex_expr + t8;
    t13 = complex_expr * 2;
    t14 = complex_expr & 0xFFFFFF;
    
    /* Switch statement for more basic blocks */
    switch (complex_expr & 0x3) {
        case 0:
            t15 = t12 + t13;
            d4 = d3 * 0.75;
            break;
        case 1:
            t15 = t13 - t12;
            d4 = d3 * 1.25;
            break;
        default:
            t15 = t12 * t13;
            d4 = d3 * 2.0;
            break;
    }
    
    /* Recomputation of similar expression later */
    int recomputed_expr = ((t11 * 3) + (t10 >> 2)) ^ (t9 & 0xF0F0) + 1;
    
    t16 = recomputed_expr | t15;
    t17 = t16 << 2;
    
    /* More floating chain */
    f7 = (float)t17 * 0.01f;
    f8 = f7 + f6;
    d5 = (double)f8 + d4;
    
    /* Final vector reduction */
    d6 = vector_reduce(dvec2);
    d7 = d5 + d6;
    
    /* Long operations */
    l4 = l3 * (long)t17;
    l5 = l4 + (long)(d7 * 1000.0);
    
    /* Final integer chain */
    t18 = t17 + (int)l5;
    t19 = t18 ^ t16;
    t20 = t19 * 2;
    
    d8 = d7 * 0.999 + (double)t20;
    
    /* Return volatile result to ensure all computations are live */
    return (long)(d8 * 1000000.0) + l5 + t20;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total_result = 0;
    
    /* Volatile inputs to prevent constant propagation */
    volatile int input1 = 42;
    volatile long input2 = 123456789L;
    volatile float input3 = 3.14159f;
    volatile double input4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Modify inputs slightly each iteration */
        volatile int modified_input1 = input1 + (i & 0xF);
        volatile long modified_input2 = input2 ^ (i * 7L);
        volatile float modified_input3 = input3 * (1.0f + i * 0.001f);
        volatile double modified_input4 = input4 / (1.0 + i * 0.0001);
        
        total_result += test_remat(modified_input1, modified_input2,
                                  modified_input3, modified_input4);
        
        /* Prevent loop unrolling */
        asm volatile("" : : : "memory");
    }
    
    printf("Final result: %ld\n", total_result);
    return (int)(total_result & 0x7FFFFFFF);
}
