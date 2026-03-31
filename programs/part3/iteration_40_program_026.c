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
    if (a > b) return a * c - b;
    else return b * c + a;
}

__attribute__((noinline, noclone))
static double helper2(double x, double y, int scale) {
    switch (scale & 3) {
        case 0: return x * y;
        case 1: return x / (y + 1.0);
        case 2: return y / (x + 1.0);
        default: return x + y;
    }
}

__attribute__((noinline, noclone))
static v4si helper3(v4si a, v4si b) {
    v4si mask = {1, -1, 2, -2};
    return a * b + (a >> 1) - (b << 1) ^ mask;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile int test_remat(volatile int input1, volatile long input2,
                               volatile float input3, volatile double input4) {
    /* Many local variables of mixed types */
    int a1 = input1 + 1;
    long b1 = input2 - 1000;
    float c1 = input3 * 2.5f;
    double d1 = input4 / 3.14159;
    int a2 = a1 * 3;
    long b2 = b1 / 7;
    float c2 = c1 + 42.0f;
    double d2 = d1 - 123.456;
    int a3 = a2 ^ 0x55AA55AA;
    long b3 = b2 | 0xFFFF0000;
    float c3 = c2 * 0.333f;
    double d3 = d2 * 2.71828;
    int a4 = a3 << 3;
    long b4 = b3 >> 2;
    float c4 = c3 / 7.0f;
    double d4 = d3 + 987.654;
    int a5 = ~a4;
    long b5 = -b4;
    float c5 = -c4;
    double d5 = -d4;
    int a6 = a5 & 0x00FF00FF;
    long b6 = b5 ^ 0xAAAAAAAA;
    float c6 = c5 + c4;
    double d6 = d5 * d4;
    int a7 = a6 | 0x55005500;
    long b7 = b6 & 0x33333333;
    float c7 = c6 - c3;
    double d7 = d6 / d3;
    int a8 = a7 + 999;
    long b8 = b7 - 8888;
    float c8 = c7 * 1.234f;
    double d8 = d7 + 3.14159;
    
    /* Vector variables */
    v4si v1 = {a1, a2, a3, a4};
    v4si v2 = {a5, a6, a7, a8};
    v4sf vf1 = {c1, c2, c3, c4};
    v4sf vf2 = {c5, c6, c7, c8};
    v2df vd1 = {d1, d2};
    v2df vd2 = {d3, d4};
    
    /* Long serial chain of interdependent operations */
    int t1 = a1 + a2;
    long t2 = b1 * b2;
    float t3 = c1 - c2;
    double t4 = d1 / d2;
    
    int t5 = t1 ^ a3;
    long t6 = t2 + b3;
    float t7 = t3 * c3;
    double t8 = t4 - d3;
    
    int t9 = t5 | a4;
    long t10 = t6 ^ b4;
    float t11 = t7 / c4;
    double t12 = t8 * d4;
    
    int t13 = t9 & a5;
    long t14 = t10 | b5;
    float t15 = t11 + c5;
    double t16 = t12 - d5;
    
    int t17 = t13 << 2;
    long t18 = t14 >> 1;
    float t19 = t15 * 1.5f;
    double t20 = t16 / 2.0;
    
    /* Vector operations */
    v4si vt1 = v1 + v2;
    v4si vt2 = v1 * v2;
    v4si vt3 = vt1 - vt2;
    v4si vt4 = helper3(vt1, vt2);
    
    v4sf vtf1 = vf1 + vf2;
    v4sf vtf2 = vf1 * vf2;
    v4sf vtf3 = vtf1 - vtf2;
    
    v2df vtd1 = vd1 + vd2;
    v2df vtd2 = vd1 * vd2;
    v2df vtd3 = vtd1 - vtd2;
    
    /* Inline assembly to clobber physical registers */
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
        "mov x16, #0\n"
        "mov x17, #0\n"
        "mov x18, #0\n"
        "mov x19, #0\n"
        "mov x20, #0\n"
        "mov x21, #0\n"
        "mov x22, #0\n"
        "mov x23, #0\n"
        "mov x24, #0\n"
        "mov x25, #0\n"
        "mov x26, #0\n"
        "mov x27, #0\n"
        "mov x28, #0\n"
        "mov x29, #0\n"
        "mov x30, #0\n"
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
        "eor v16.16b, v16.16b, v16.16b\n"
        "eor v17.16b, v17.16b, v17.16b\n"
        "eor v18.16b, v18.16b, v18.16b\n"
        "eor v19.16b, v19.16b, v19.16b\n"
        "eor v20.16b, v20.16b, v20.16b\n"
        "eor v21.16b, v21.16b, v21.16b\n"
        "eor v22.16b, v22.16b, v22.16b\n"
        "eor v23.16b, v23.16b, v23.16b\n"
        "eor v24.16b, v24.16b, v24.16b\n"
        "eor v25.16b, v25.16b, v25.16b\n"
        "eor v26.16b, v26.16b, v26.16b\n"
        "eor v27.16b, v27.16b, v27.16b\n"
        "eor v28.16b, v28.16b, v28.16b\n"
        "eor v29.16b, v29.16b, v29.16b\n"
        "eor v30.16b, v30.16b, v30.16b\n"
        "eor v31.16b, v31.16b, v31.16b\n"
        :
        :
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7",
          "x8", "x9", "x10", "x11", "x12", "x13", "x14", "x15",
          "x16", "x17", "x18", "x19", "x20", "x21", "x22", "x23",
          "x24", "x25", "x26", "x27", "x28", "x29", "x30",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7",
          "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15",
          "v16", "v17", "v18", "v19", "v20", "v21", "v22", "v23",
          "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
          "memory"
#else
        /* Generic clobber for other architectures */
        "memory"
#endif
    );
    
    /* More operations after assembly clobber */
    int t21 = t17 + t13;
    long t22 = t18 * t14;
    float t23 = t19 - t15;
    double t24 = t20 / t16;
    
    /* Complex expression that might be recomputed */
    int complex1 = (t1 * t5 + t9 / 3) ^ (t13 & 0xFF);
    long complex2 = (t2 - t6) | (t10 ^ 0x5555);
    
    /* Use complex expression multiple times */
    int u1 = complex1 + a1;
    int u2 = complex1 - a2;
    int u3 = complex1 * a3;
    
    long v1_l = complex2 + b1;
    long v2_l = complex2 - b2;
    long v3_l = complex2 * b3;
    
    /* Control flow to split basic blocks */
    if (u1 > u2) {
        u1 = helper1(u1, u2, u3);
        if (v1_l < v2_l) {
            v1_l = helper1(v1_l, v2_l, v3_l);
        }
    } else {
        u2 = helper1(u2, u3, u1);
    }
    
    /* Recomputation of similar complex expression */
    int complex1_again = (t1 * t5 + t9 / 3) ^ (t13 & 0xFF) + 1;  /* Slightly different */
    long complex2_again = (t2 - t6) | (t10 ^ 0x5555) << 1;       /* Slightly different */
    
    /* More vector operations */
    v4si vt5 = vt3 + vt4;
    v4si vt6 = vt3 * vt4;
    v4sf vtf4 = vtf3 * 2.0f;
    v2df vtd4 = vtd3 / 2.0;
    
    /* Use helper functions with control flow */
    double d_result = helper2(d4, d5, complex1);
    v4si v_result = helper3(vt5, vt6);
    
    /* Final computation using all temporaries */
    volatile int result = 
        (t1 + t5 + t9 + t13 + t17 + t21) ^ 
        (u1 + u2 + u3) |
        (complex1 + complex1_again) &
        (v_result[0] + v_result[1] + v_result[2] + v_result[3]) +
        (int)(t3 + t7 + t11 + t15 + t19 + t23) +
        (int)(t4 + t8 + t12 + t16 + t20 + t24) +
        (int)d_result;
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    volatile int input_seed = 42;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        volatile int result = test_remat(
            input_seed + i,
            input_seed * 3L - i,
            input_seed * 0.5f + i * 0.1f,
            input_seed * 1.5 + i * 0.01
        );
        total += result;
        
        /* Additional control flow */
        if (i % 10 == 0) {
            total ^= 0x12345678;
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total & 0x7FFFFFFF);
}
