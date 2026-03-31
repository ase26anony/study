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
static int compute_partial(int a, int b, int c) {
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
        return sum * 0.9;
    } else {
        return sum * 1.1;
    }
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile long test_remat(volatile int loop_count) {
    /* Many local variables of mixed types */
    volatile int a1 = 1, a2 = 2, a3 = 3, a4 = 4, a5 = 5;
    volatile long b1 = 10, b2 = 20, b3 = 30, b4 = 40, b5 = 50;
    volatile float c1 = 1.1f, c2 = 2.2f, c3 = 3.3f, c4 = 4.4f, c5 = 5.5f;
    volatile double d1 = 10.1, d2 = 20.2, d3 = 30.3, d4 = 40.4, d5 = 50.5;
    volatile int e1 = 100, e2 = 200, e3 = 300, e4 = 400, e5 = 500;
    volatile long f1 = 1000, f2 = 2000, f3 = 3000, f4 = 4000, f5 = 5000;
    volatile float g1 = 100.1f, g2 = 200.2f, g3 = 300.3f, g4 = 400.4f, g5 = 500.5f;
    volatile double h1 = 1000.1, h2 = 2000.2, h3 = 3000.3, h4 = 4000.4, h5 = 5000.5;
    
    /* Vector variables */
    v4si vec1 = {1, 2, 3, 4};
    v4si vec2 = {5, 6, 7, 8};
    v4sf vecf1 = {1.1f, 2.2f, 3.3f, 4.4f};
    v4sf vecf2 = {5.5f, 6.6f, 7.7f, 8.8f};
    v2df vecd1 = {100.1, 200.2};
    v2df vecd2 = {300.3, 400.4};
    
    /* Long chain of interdependent computations */
    long t1 = a1 + b1;
    double t2 = c1 * d1;
    float t3 = g1 - h1;
    v4si t4 = vec1 + vec2;
    long t5 = t1 * a2;
    double t6 = t2 + d2;
    float t7 = t3 * c2;
    v4sf t8 = vecf1 * vecf2;
    long t9 = t5 - b2;
    double t10 = t6 / d3;
    float t11 = t7 + g2;
    v2df t12 = vecd1 + vecd2;
    
    /* Complex expression computed and used multiple times */
    long complex_expr = (a3 * b3) + (e1 >> 2) - (f1 & 0xFF);
    double complex_double = (d4 * h4) / (c4 + 1.0f);
    
    /* Use complex expression in multiple statements */
    long use1 = complex_expr * t9;
    double use2 = complex_double + t10;
    long use3 = complex_expr / 2 + t1;
    double use4 = complex_double * 0.5 - t2;
    
    /* Inline assembly to clobber registers */
    asm volatile (
#if defined(__x86_64__)
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
#elif defined(__arm__) || defined(__aarch64__)
        "mov x0, #0\n"
        "mov x1, #0\n"
        "mov x2, #0\n"
        "mov x3, #0\n"
        "mov x4, #0\n"
        "mov x5, #0\n"
        "mov x6, #0\n"
        "mov x7, #0\n"
        "mov x8, #0\n"
        "mov x9, #0\n"
        "mov x10, #0\n"
        "mov x11, #0\n"
        "mov x12, #0\n"
        "mov x13, #0\n"
        "mov x14, #0\n"
        "mov x15, #0\n"
        "eor v0.16b, v0.16b, v0.16b\n"
        "eor v1.16b, v1.16b, v1.16b\n"
        "eor v2.16b, v2.16b, v2.16b\n"
        "eor v3.16b, v3.16b, v3.16b\n"
        "eor v4.16b, v4.16b, v4.16b\n"
        "eor v5.16b, v5.16b, v5.16b\n"
        "eor v6.16b, v6.16b, v6.16b\n"
        "eor v7.16b, v7.16b, v7.16b\n"
        "eor v8.16b, v8.16b, v8.16b\n"
        "eor v9.16b, v9.16b, v9.16b\n"
        "eor v10.16b, v10.16b, v10.16b\n"
        "eor v11.16b, v11.16b, v11.16b\n"
        "eor v12.16b, v12.16b, v12.16b\n"
        "eor v13.16b, v13.16b, v13.16b\n"
        "eor v14.16b, v14.16b, v14.16b\n"
        "eor v15.16b, v15.16b, v15.16b\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "memory"
#else
        "" ::: "memory"
#endif
    );
    
    /* More computations after clobber */
    long t13 = use1 + a4;
    double t14 = use2 - d5;
    float t15 = t11 * g3;
    v4si t16 = t4 * vec1;
    long t17 = t13 / b4;
    double t18 = t14 * h5;
    float t19 = t15 + c5;
    v4sf t20 = t8 + vecf1;
    
    /* Control flow to split basic blocks */
    int selector = loop_count & 7;
    long result = 0;
    
    switch (selector) {
        case 0:
            result = t1 + t5 + t9 + t13 + t17;
            break;
        case 1:
            result = t1 * 2 - t5 + t9 / 2;
            break;
        case 2:
            result = compute_partial(t1, t5, t9);
            break;
        case 3:
            result = t13 ^ t17 | t9;
            break;
        case 4:
            result = (t1 << 3) | (t5 >> 2);
            break;
        case 5:
            result = t17 * 3 - t13;
            break;
        case 6:
            result = compute_partial(t13, t17, t9);
            break;
        default:
            result = t1 + t13 + t17;
            break;
    }
    
    /* Recomputation of complex expression in different form */
    long complex_expr2 = (a3 * b3) + (e1 >> 2) - (f1 & 0xFF) + loop_count;
    double complex_double2 = (d4 * h4) / (c4 + 1.0f) * 2.0;
    
    /* Use recomputed values */
    long final1 = complex_expr2 + result;
    double final2 = complex_double2 + vector_reduce(t12);
    
    /* Vector operations mixed with scalar */
    v4si vec3 = t16 + vec2;
    v4sf vecf3 = t20 * 2.0f;
    v2df vecd3 = t12 * 1.5;
    
    /* Final aggregation */
    long vec_sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
    float vecf_sum = vecf3[0] + vecf3[1] + vecf3[2] + vecf3[3];
    double vecd_sum = vector_reduce(vecd3);
    
    /* Return volatile result to ensure all computations are live */
    volatile long final_result = final1 + (long)final2 + vec_sum + 
                                 (long)vecf_sum + (long)vecd_sum;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    volatile long total = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        volatile long result = test_remat(i);
        total += result;
        
        /* Additional control flow to prevent loop optimizations */
        if (i % 100 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total & 0x7FFFFFFF);
}
