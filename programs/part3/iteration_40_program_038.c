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
        return (a + b) * c / 2;
    }
}

__attribute__((noinline, noclone))
static double vector_compute(v4si vi, v4sf vf, v2df vd) {
    /* Force vector operations across multiple basic blocks */
    v4si vi2 = vi + (v4si){1, 2, 3, 4};
    v4sf vf2 = vf * (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
    v2df vd2 = vd + (v2df){0.5, 1.5};
    
    switch (vi[0] & 3) {
        case 0:
            return (double)vi2[0] + vf2[0] + vd2[0];
        case 1:
            return (double)vi2[1] + vf2[1] + vd2[1];
        case 2:
            return (double)vi2[2] + vf2[2] + vd2[0];
        default:
            return (double)vi2[3] + vf2[3] + vd2[1];
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
    
    /* More variables for register pressure */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5, f6;
    double d1, d2, d3, d4, d5, d6, d7;
    
    /* Vector variables */
    v4si vec_i1 = {a, a+1, a+2, a+3};
    v4si vec_i2 = {b&0xFF, (b>>8)&0xFF, (b>>16)&0xFF, (b>>24)&0xFF};
    v4sf vec_f1 = {c, c*2.0f, c*3.0f, c*4.0f};
    v4sf vec_f2 = {c+1.0f, c+2.0f, c+3.0f, c+4.0f};
    v2df vec_d1 = {d, d*2.0};
    v2df vec_d2 = {d+1.0, d+2.0};
    
    /* Long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (int)c;
    t3 = t2 - a;
    t4 = t3 ^ (t1 << 2);
    t5 = t4 | (t2 >> 1);
    t6 = t5 & 0x7FFFFFFF;
    t7 = t6 + t3 - t1;
    t8 = t7 * 3;
    t9 = t8 / 2;
    t10 = t9 % 1000;
    
    l1 = b + t1;
    l2 = l1 * t2;
    l3 = l2 - b;
    l4 = l3 ^ (l1 << 3);
    l5 = l4 | (l2 >> 2);
    
    f1 = c + t1;
    f2 = f1 * t2;
    f3 = f2 - c;
    f4 = f3 / (f1 + 1.0f);
    f5 = f4 * 2.5f;
    f6 = f5 - f3 + f2;
    
    d1 = d + t1;
    d2 = d1 * t2;
    d3 = d2 - d;
    d4 = d3 / (d1 + 1.0);
    d5 = d4 * 3.14159;
    d6 = d5 - d3 + d2;
    d7 = d6 * 2.71828;
    
    /* Vector operations */
    v4si vec_i3 = vec_i1 + vec_i2;
    v4si vec_i4 = vec_i3 * (v4si){t1, t2, t3, t4};
    v4sf vec_f3 = vec_f1 + vec_f2;
    v4sf vec_f4 = vec_f3 * (v4sf){f1, f2, f3, f4};
    v2df vec_d3 = vec_d1 + vec_d2;
    v2df vec_d4 = vec_d3 * (v2df){d1, d2};
    
    /* Inline assembly to clobber physical registers */
    /* x86_64 version - adjust for your architecture */
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
        "pxor %%xmm15, %%xmm15\n\t"
        :
        :
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
          "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
          "memory"
    );
    
    /* Complex expression computed and used multiple times */
    /* This pattern encourages rematerialization */
    int complex_expr = (t1 * t2 + t3 - t4) ^ (t5 | t6) & (t7 << 3);
    
    /* Use complex_expr in multiple statements */
    int use1 = complex_expr + t8;
    int use2 = complex_expr - t9;
    int use3 = complex_expr * t10;
    
    /* Recompute similar expression later in same iteration */
    /* This might trigger early rematerialization decisions */
    int complex_expr2 = (t1 * t2 + t3 - t4) ^ (t5 | t6) & (t7 << 3) + 1;
    
    /* More operations using the recomputed value */
    int use4 = complex_expr2 + use1;
    int use5 = complex_expr2 - use2;
    int use6 = complex_expr2 * use3;
    
    /* Control flow to split basic blocks */
    if (use1 > use2) {
        t1 = compute_branch(use1, use2, use3);
        vec_i1 = vec_i1 + (v4si){t1, t1+1, t1+2, t1+3};
    } else {
        t2 = compute_branch(use2, use1, use3);
        vec_i2 = vec_i2 + (v4si){t2, t2-1, t2-2, t2-3};
    }
    
    /* More vector operations in different basic blocks */
    switch (use3 & 7) {
        case 0:
            vec_f3 = vec_f1 * 2.0f;
            break;
        case 1:
            vec_f3 = vec_f2 / 2.0f;
            break;
        case 2:
            vec_f3 = vec_f1 + vec_f2;
            break;
        case 3:
            vec_f3 = vec_f1 - vec_f2;
            break;
        default:
            vec_f3 = (vec_f1 + vec_f2) * 0.5f;
            break;
    }
    
    /* Final computation using all temporaries */
    double vec_result = vector_compute(vec_i1, vec_f1, vec_d1);
    
    volatile double final_result = 
        (double)t1 + (double)t2 + (double)t3 + (double)t4 + (double)t5 +
        (double)t6 + (double)t7 + (double)t8 + (double)t9 + (double)t10 +
        (double)l1 + (double)l2 + (double)l3 + (double)l4 + (double)l5 +
        (double)f1 + (double)f2 + (double)f3 + (double)f4 + (double)f5 + (double)f6 +
        d1 + d2 + d3 + d4 + d5 + d6 + d7 +
        vec_result +
        (double)use1 + (double)use2 + (double)use3 +
        (double)use4 + (double)use5 + (double)use6 +
        (double)complex_expr + (double)complex_expr2;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 1000;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 1000;
    }
    
    volatile double total = 0.0;
    volatile int seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int in1 = seed + i;
        volatile long in2 = seed * 3 + i * 7;
        volatile float in3 = (seed + i) * 0.5f;
        volatile double in4 = (seed + i) * 0.25;
        
        double result = test_remat(in1, in2, in3, in4);
        total += result;
        
        /* Modify seed to change computation pattern */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Final result: %f\n", total);
    return 0;
}
