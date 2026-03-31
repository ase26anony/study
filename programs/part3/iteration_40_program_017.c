/* early_remat_test.c - Target GCC's early rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    double sum = 0.0;
    for (int i = 0; i < 4; i++) {
        sum += v[i];
    }
    return sum;
}

/* Main test function with high register pressure */
__attribute__((noinline, noclone))
static volatile double test_remat(volatile int input1, volatile long input2,
                                  volatile float input3, volatile double input4) {
    /* Declare many local variables - at least 30 */
    volatile int a1 = input1 + 1;
    volatile long b1 = input2 - 2;
    volatile float c1 = input3 * 1.5f;
    volatile double d1 = input4 / 2.0;
    
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    long l1, l2, l3, l4, l5, l6, l7, l8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    double d2, d3, d4, d5, d6, d7, d8, d9;
    
    /* Vector variables */
    v4si vec1 = {a1, a1 + 1, a1 + 2, a1 + 3};
    v4si vec2 = {5, 6, 7, 8};
    v4sf vecf1 = {c1, c1 * 2.0f, c1 * 3.0f, c1 * 4.0f};
    v4sf vecf2 = {1.1f, 2.2f, 3.3f, 4.4f};
    v2df vecd1 = {d1, d1 * 1.5};
    v2df vecd2 = {2.5, 3.5};
    
    /* Complex serial chain of interdependent operations */
    t1 = a1 * 3 + (int)b1;
    t2 = t1 - a1 * 2;
    t3 = t2 * t1 + 7;
    t4 = t3 / (t2 + 1) | 0xFF;
    t5 = (t4 ^ t3) & 0xFFFF;
    t6 = t5 << 3;
    t7 = t6 >> 1;
    t8 = t7 * t5 - t4;
    t9 = t8 % 257;
    t10 = t9 * t7 + t6;
    
    l1 = b1 * 3L + t1;
    l2 = l1 - b1 * 2L;
    l3 = l2 * l1 + 1000L;
    l4 = l3 / (l2 + 1L) | 0xFF00L;
    l5 = (l4 ^ l3) & 0xFFFFFFL;
    l6 = l5 << 5;
    l7 = l6 >> 2;
    l8 = l7 * l5 - l4;
    
    f1 = c1 * 3.0f + t2;
    f2 = f1 - c1 * 2.0f;
    f3 = f2 * f1 + 10.5f;
    f4 = f3 / (f2 + 1.0f) + 0.25f;
    f5 = f4 * f3 - f2;
    f6 = f5 / 1.7f + f4;
    f7 = f6 * 2.3f - f5;
    f8 = f7 / 0.7f + f6;
    
    d2 = d1 * 3.0 + l1;
    d3 = d2 - d1 * 2.0;
    d4 = d3 * d2 + 20.5;
    d5 = d4 / (d3 + 1.0) + 0.125;
    d6 = d5 * d4 - d3;
    d7 = d6 / 1.9 + d5;
    d8 = d7 * 2.7 - d6;
    d9 = d8 / 0.9 + d7;
    
    /* Vector operations */
    v4si vec3 = vec1 + vec2;
    v4si vec4 = vec3 * vec1 - vec2;
    v4si vec5 = vec4 << 1;
    v4si vec6 = vec5 >> 1 | vec4;
    
    v4sf vecf3 = vecf1 + vecf2;
    v4sf vecf4 = vecf3 * vecf1 - vecf2;
    v4sf vecf5 = vecf4 * 1.5f;
    v4sf vecf6 = vecf5 / 0.75f + vecf4;
    
    v2df vecd3 = vecd1 + vecd2;
    v2df vecd4 = vecd3 * vecd1 - vecd2;
    v2df vecd5 = vecd4 * 1.25;
    v2df vecd6 = vecd5 / 0.8 + vecd4;
    
    /* Inline assembly to clobber registers - x86_64 version */
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
          "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
          "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
          "xmm12", "xmm13", "xmm14", "xmm15", "memory"
    );
    
    /* Control flow to create multiple basic blocks */
    int branch_result;
    if (t10 > 1000) {
        branch_result = compute_branch(t10, t9, t8);
        vec3 = vec3 + (v4si){branch_result, 0, 0, 0};
    } else if (t10 > 500) {
        branch_result = compute_branch(t9, t8, t7);
        vec3 = vec3 - (v4si){0, branch_result, 0, 0};
    } else {
        branch_result = compute_branch(t8, t7, t6);
        vec3 = vec3 | (v4si){0, 0, branch_result, 0};
    }
    
    /* Switch statement for more control flow */
    switch (t10 % 5) {
        case 0:
            vecf3 = vecf3 * 1.1f;
            d9 += 1.0;
            break;
        case 1:
            vecf3 = vecf3 / 1.1f;
            d9 -= 1.0;
            break;
        case 2:
            vecf3 = vecf3 + vecf4;
            d9 *= 1.05;
            break;
        case 3:
            vecf3 = vecf3 - vecf4;
            d9 /= 1.05;
            break;
        default:
            vecf3 = vecf3 * vecf4;
            d9 = d9 * d8;
            break;
    }
    
    /* Recomputation of earlier values in different forms */
    /* This increases likelihood of early rematerialization */
    int t1_recomp = a1 * 3 + (int)b1;  /* Same as t1 */
    int t2_recomp = t1_recomp - a1 * 2 + 1;  /* Similar to t2 but different */
    long l1_recomp = b1 * 3L + t1_recomp;  /* Same as l1 */
    float f1_recomp = c1 * 3.0f + t2_recomp;  /* Similar to f1 */
    double d2_recomp = d1 * 3.0 + l1_recomp;  /* Same as d2 */
    
    /* Use recomputed values in new expressions */
    int t11 = t1_recomp * t2_recomp - t10;
    long l9 = l1_recomp * 2 - l8;
    float f9 = f1_recomp * 2.0f - f8;
    double d10 = d2_recomp * 2.0 - d9;
    
    /* More vector operations with recomputed values */
    v4si vec7 = vec3 + (v4si){t11, t11/2, t11/3, t11/4};
    v4sf vecf7 = vecf3 + (v4sf){f9, f9/2.0f, f9/3.0f, f9/4.0f};
    v2df vecd7 = vecd3 + (v2df){d10, d10/2.0};
    
    /* Final reduction using all variables */
    double final_sum = 0.0;
    final_sum += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 + t11;
    final_sum += l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9;
    final_sum += f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9;
    final_sum += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
    
    /* Vector reductions */
    for (int i = 0; i < 4; i++) {
        final_sum += vec3[i] + vec4[i] + vec5[i] + vec6[i] + vec7[i];
        final_sum += vecf3[i] + vecf4[i] + vecf5[i] + vecf6[i] + vecf7[i];
    }
    for (int i = 0; i < 2; i++) {
        final_sum += vecd3[i] + vecd4[i] + vecd5[i] + vecd6[i] + vecd7[i];
    }
    
    final_sum += vector_reduce(vecf6);
    final_sum += branch_result;
    
    return final_sum;
}

int main(int argc, char *argv[]) {
    volatile int loop_count = 100;
    if (argc > 1) {
        loop_count = atoi(argv[1]);
        if (loop_count <= 0) loop_count = 100;
    }
    
    volatile double total = 0.0;
    volatile int seed1 = 42;
    volatile long seed2 = 123456789L;
    volatile float seed3 = 3.14159f;
    volatile double seed4 = 2.718281828459045;
    
    for (volatile int i = 0; i < loop_count; i++) {
        /* Vary inputs slightly each iteration */
        double result = test_remat(
            seed1 + i,
            seed2 + i * 100L,
            seed3 + i * 0.1f,
            seed4 + i * 0.01
        );
        total += result;
        
        /* Modify seeds to change computation pattern */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = seed2 * 6364136223846793005L + 1442695040888963407L;
        seed3 = seed3 * 1.1f + 0.5f;
        seed4 = seed4 * 1.01 + 0.001;
    }
    
    printf("Final result: %f\n", (double)total);
    return 0;
}
