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
static volatile int test_remat(volatile int input1, volatile long input2, 
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
    
    /* Vector variables */
    v4si vec1, vec2, vec3, vec4;
    v4sf fvec1, fvec2, fvec3;
    v2df dvec1, dvec2;
    
    /* Initialize vectors */
    vec1 = (v4si){a, a+1, a+2, a+3};
    vec2 = (v4si){b%10, b%20, b%30, b%40};
    fvec1 = (v4sf){c, c*2, c*3, c*4};
    dvec1 = (v2df){d, d*1.5};
    
    /* Start long serial chain of interdependent operations */
    t1 = a + (int)b;
    t2 = t1 * (a % 7);
    t3 = t2 - (b % 11);
    t4 = t3 ^ t1;
    t5 = t4 | (t2 & 0xFF);
    t6 = t5 << 3;
    t7 = t6 >> 1;
    t8 = t7 + t3 - t1;
    t9 = t8 * 137;
    t10 = t9 / (a > 0 ? a : 1);
    
    /* Floating point chain */
    f1 = c * 2.5f;
    f2 = f1 + (float)d;
    f3 = f2 * c;
    f4 = f3 - f1;
    f5 = f4 / (c + 1.0f);
    f6 = f5 * f2;
    f7 = f6 - f3;
    f8 = f7 + f4;
    f9 = f8 * 0.75f;
    f10 = f9 / (f1 + 0.001f);
    
    /* Double precision chain */
    d1 = d * 3.14159;
    d2 = d1 + (double)a;
    d3 = d2 * 1.618;
    d4 = d3 - d1;
    d5 = d4 / (d + 0.0001);
    d6 = d5 * d2;
    d7 = d6 - d3;
    d8 = d7 + d4;
    d9 = d8 * 0.33333;
    d10 = d9 / (d1 + 0.00001);
    
    /* Vector operations - consume wide registers */
    vec3 = vec1 + vec2;
    vec4 = vec3 * vec1;
    vec4 = vec4 - vec2;
    vec4 = vec4 & vec3;
    
    fvec2 = fvec1 * (v4sf){f1, f2, f3, f4};
    fvec3 = fvec2 + fvec1;
    fvec3 = fvec3 - (v4sf){0.5f, 1.5f, 2.5f, 3.5f};
    
    dvec2 = dvec1 * (v2df){d1, d2};
    dvec2 = dvec2 + (v2df){d3, d4};
    
    /* Inline assembly to clobber physical registers */
    /* For x86_64 */
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
    );
    
    /* For ARM/AArch64, use this instead:
    asm volatile (
        "# Clobber ARM registers\n"
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
          "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29", "x30",
          "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9",
          "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18", "v19",
          "v20", "v21", "v22", "v23", "v24", "v25", "v26", "v27", "v28", "v29", "v30", "v31",
          "memory"
    );
    */
    
    /* More computations after clobber - forces reloads/rematerialization */
    int t21 = t10 + t5;
    int t22 = t21 * (t3 % 13);
    int t23 = t22 - t8;
    int t24 = t23 ^ t21;
    int t25 = t24 | (t22 & 0x7F);
    
    /* Complex expression computed, used, then recomputed differently */
    /* This pattern encourages early rematerialization */
    int complex_expr = (t10 * 3 + t5 / 2 - t8 % 7) ^ (t3 << 2);
    
    /* Use complex_expr multiple times */
    t11 = complex_expr + t21;
    t12 = complex_expr - t22;
    t13 = complex_expr * t23;
    
    /* Control flow to split basic blocks */
    if (complex_expr > 1000) {
        t14 = compute_branch(complex_expr, t11, t12);
    } else if (complex_expr < -1000) {
        t14 = compute_branch(t12, t13, complex_expr);
    } else {
        t14 = complex_expr * 2;
    }
    
    /* Recomputation of similar expression */
    int complex_expr2 = (t10 * 3 + t5 / 2 - t8 % 7) ^ (t3 << 2) + 1;
    
    t15 = complex_expr2 + t24;
    t16 = complex_expr2 - t25;
    
    /* More floating point after clobber */
    float f11 = f10 * 2.0f;
    float f12 = f11 + (float)t14;
    float f13 = f12 * c;
    
    double d11 = d10 * 2.0;
    double d12 = d11 + (double)t15;
    double d13 = d12 * d;
    
    /* Vector operations after clobber */
    v4si vec5 = vec4 + vec3;
    vec5 = vec5 * (v4si){t11, t12, t13, t14};
    
    /* Switch statement for more control flow */
    switch (t14 % 5) {
        case 0:
            t17 = t15 + t16;
            break;
        case 1:
            t17 = t15 - t16;
            break;
        case 2:
            t17 = t15 * t16;
            break;
        case 3:
            t17 = t15 / (t16 != 0 ? t16 : 1);
            break;
        default:
            t17 = t15 ^ t16;
            break;
    }
    
    /* Final mixing of all values */
    t18 = t17 + (int)f13;
    t19 = t18 + (int)d13;
    
    /* Reduce vectors to scalars */
    int vec_sum = vec5[0] + vec5[1] + vec5[2] + vec5[3];
    double fvec_avg = vector_reduce(fvec3);
    
    t20 = t19 + vec_sum + (int)fvec_avg;
    
    /* Return volatile result to ensure all computations are live */
    volatile int result = t20 + (int)(dvec2[0] * 100) + (int)(dvec2[1] * 100);
    
    return result;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile long total = 0;
    
    /* Loop to increase rematerialization opportunities */
    for (volatile int i = 0; i < loop_count; i++) {
        /* Use different inputs each iteration */
        volatile int input1 = i * 3 + 1;
        volatile long input2 = i * 5 + 2;
        volatile float input3 = i * 1.5f + 3.0f;
        volatile double input4 = i * 2.5 + 4.0;
        
        volatile int result = test_remat(input1, input2, input3, input4);
        total += result;
        
        /* Small conditional to prevent loop unrolling */
        if (i % 7 == 0) {
            total -= result / 2;
        }
    }
    
    printf("Final result: %ld\n", (long)total);
    return (int)(total % 1000);
}
