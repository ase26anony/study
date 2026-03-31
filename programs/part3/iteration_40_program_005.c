/* early-remat-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    return a + b * 3;
}

__attribute__((noinline, noclone))
static v4sf helper4(v4sf a, v4sf b) {
    return a * b + (v4sf){1.0f, 2.0f, 3.0f, 4.0f};
}

__attribute__((noinline, noclone))
static long complex_chain_1(volatile int input, long base) {
    /* Many local variables to create register pressure */
    int v1 = input + 1;
    int v2 = input * 2;
    int v3 = v1 ^ v2;
    int v4 = v3 << 3;
    int v5 = v4 | 0xFF;
    int v6 = v5 - v1;
    int v7 = v6 * 7;
    int v8 = v7 / 2;
    int v9 = v8 & 0x7F;
    int v10 = v9 + 12345;
    
    long l1 = base + v1;
    long l2 = l1 * v2;
    long l3 = l2 - v3;
    long l4 = l3 ^ v4;
    long l5 = l4 | v5;
    long l6 = l5 & 0xFFFFFFFF;
    long l7 = l6 * 13;
    long l8 = l7 + v6;
    long l9 = l8 - v7;
    long l10 = l9 ^ v8;
    
    float f1 = v1 * 0.1f;
    float f2 = v2 * 0.2f;
    float f3 = f1 + f2;
    float f4 = f3 * 3.14f;
    float f5 = f4 - 1.618f;
    float f6 = f5 / 2.718f;
    float f7 = f6 + f1;
    float f8 = f7 * f2;
    float f9 = f8 - f3;
    float f10 = f9 / f4;
    
    double d1 = v3 * 0.01;
    double d2 = v4 * 0.02;
    double d3 = d1 + d2;
    double d4 = d3 * 1.414;
    double d5 = d4 - 2.718;
    double d6 = d5 / 3.141;
    double d7 = d6 + d1;
    double d8 = d7 * d2;
    double d9 = d8 - d3;
    double d10 = d9 / d4;
    
    /* Vector operations */
    v4si vec1 = {v1, v2, v3, v4};
    v4si vec2 = {v5, v6, v7, v8};
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec1 * vec2;
    v4si vec5 = vec3 - vec4;
    v4si vec6 = vec5 << 1;
    
    v4sf vecf1 = {f1, f2, f3, f4};
    v4sf vecf2 = {f5, f6, f7, f8};
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf1 * vecf2;
    v4sf vecf5 = vecf3 - vecf4;
    
    v2df vecd1 = {d1, d2};
    v2df vecd2 = {d3, d4};
    v2df vecd3 = vecd1 + vecd2;
    v2df vecd4 = vecd1 * vecd2;
    v2df vecd5 = vecd3 - vecd4;
    
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
        : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11",
          "r12", "r13", "r14", "r15", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4",
          "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12",
          "xmm13", "xmm14", "xmm15", "memory"
#elif defined(__aarch64__)
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
        : "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
          "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
          "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29",
          "x30", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8",
          "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18",
          "v19", "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28",
          "v29", "v30", "v31", "memory"
#else
        #error "Unsupported architecture"
#endif
    );
    
    /* Complex expression that might be rematerialized */
    int complex_expr = (v1 * v2 + v3 - v4) ^ (v5 | v6) & (v7 ^ v8);
    
    /* Use complex_expr multiple times */
    int use1 = complex_expr + v9;
    int use2 = complex_expr - v10;
    int use3 = complex_expr * v1;
    
    /* Control flow to split basic blocks */
    if (complex_expr > 1000) {
        use1 = helper1(use1, use2, use3);
    } else {
        use1 = helper1(use3, use2, use1);
    }
    
    switch (complex_expr & 0x7) {
        case 0: use2 = helper1(use2, use1, v2); break;
        case 1: use2 = helper1(use1, use2, v3); break;
        case 2: use2 = helper1(v4, use2, use1); break;
        case 3: use2 = helper1(use2, v5, use1); break;
        case 4: use2 = helper1(use1, v6, use2); break;
        case 5: use2 = helper1(v7, use1, use2); break;
        case 6: use2 = helper1(use2, use1, v8); break;
        default: use2 = helper1(use1, use2, v9); break;
    }
    
    /* Recomputation of similar expression */
    int recomputed = (v1 * v2 + v3 - v4) ^ (v5 | v6) & (v7 ^ v8) + 1;
    
    /* More operations with recomputed value */
    long result = (long)use1 + (long)use2 + (long)recomputed;
    result += l10 + (long)(f10 * 100.0f) + (long)(d10 * 1000.0);
    
    /* Extract elements from vectors */
    result += vec6[0] + vec6[1] + vec6[2] + vec6[3];
    result += (long)(vecf5[0] * 10.0f) + (long)(vecf5[1] * 20.0f);
    result += (long)(vecd5[0] * 100.0) + (long)(vecd5[1] * 200.0);
    
    return result;
}

__attribute__((noinline, noclone))
static volatile long test_remat(volatile int input) {
    volatile long result = 0;
    volatile int loop_counter = input & 0x3F; /* Limit to 0-63 iterations */
    
    for (volatile int i = 0; i < loop_counter; i++) {
        /* Each iteration creates fresh register pressure */
        long iter_result = complex_chain_1(input + i, result);
        
        /* Complex expression computed and used multiple times */
        int expr = (input * i + (i << 3)) ^ (input | i) & (input ^ i);
        
        /* Multiple uses of expr */
        int use_a = expr + input;
        int use_b = expr - i;
        int use_c = expr * input;
        
        /* Control flow */
        if (expr > 100) {
            use_a = helper1(use_a, use_b, use_c);
        } else {
            use_a = helper1(use_c, use_b, use_a);
        }
        
        /* Recomputation of similar expression */
        int expr2 = (input * i + (i << 3)) ^ (input | i) & (input ^ i) + 1;
        
        /* Mix with vector operations */
        v4si vec_a = {use_a, use_b, use_c, expr2};
        v4si vec_b = {input, i, expr, expr2};
        v4si vec_c = helper3(vec_a, vec_b);
        
        /* Floating point vector operations */
        v4sf vec_f1 = {(float)use_a, (float)use_b, (float)use_c, (float)expr2};
        v4sf vec_f2 = {(float)input, (float)i, (float)expr, (float)expr2};
        v4sf vec_f3 = helper4(vec_f1, vec_f2);
        
        /* Accumulate results */
        result += iter_result + (long)use_a + (long)use_b + (long)expr2;
        result += vec_c[0] + vec_c[1] + vec_c[2] + vec_c[3];
        result += (long)(vec_f3[0] * 100.0f) + (long)(vec_f3[1] * 200.0f);
        
        /* Another inline assembly to clobber registers mid-loop */
        asm volatile ("" ::: "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 1000) iterations = 1000;
    }
    
    volatile long total_result = 0;
    
    for (volatile int i = 0; i < iterations; i++) {
        volatile long result = test_remat(i);
        total_result += result;
        
        /* Prevent optimization */
        asm volatile ("" : "+r" (total_result));
    }
    
    printf("Result: %ld\n", (long)total_result);
    return 0;
}
