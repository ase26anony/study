#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force register spilling with many live variables */
int main(int argc, char **argv) {
    /* Integer variables - will compete for general purpose registers */
    volatile int v1 = argc + 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc * 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc * 7;
    volatile int v8 = argc * 8;
    volatile int v9 = argc * 9;
    volatile int v10 = argc * 10;
    
    /* Floating point variables - will compete for FP/vector registers */
    volatile float f1 = v1 * 1.1f;
    volatile float f2 = v2 * 1.2f;
    volatile float f3 = v3 * 1.3f;
    volatile float f4 = v4 * 1.4f;
    volatile float f5 = v5 * 1.5f;
    
    /* Double precision variables */
    volatile double d1 = v1 * 1.11;
    volatile double d2 = v2 * 1.22;
    volatile double d3 = v3 * 1.33;
    volatile double d4 = v4 * 1.44;
    
    /* Vector variables - use many vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f5};
    v4sf vec3 = {f3, f4, f5, f1};
    v4sf vec4 = {f4, f5, f1, f2};
    
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d3, d4};
    
    v4si ivec1 = {v1, v2, v3, v4};
    v4si ivec2 = {v5, v6, v7, v8};
    
    /* Pointer variables - more register pressure */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    volatile double *dp1 = &d1;
    
    /* Loop to create complex control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile long long accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional to create branching */
        if (i % 2 == 0) {
            /* Path 1: Heavy computation before call */
            vec1 = vec1 + vec2 * vec3;
            vec4 = vec4 - vec1;
            dvec1 = dvec1 * 1.5 + dvec2;
            
            /* Integer computations keeping many vars live */
            v1 = v2 + v3 * v4 - v5;
            v6 = v7 ^ v8 | v9;
            v10 = v1 * v2 + v3;
            
            /* Force specific register clobbering before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* More clobbering after call */
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Continue computations with live variables */
            vec2 = vec1 * 2.0f - vec4;
            f1 = f2 + f3 * f4;
            d2 = d1 * d3 - d4;
            
            /* Pointer operations */
            *p1 = *p2 + v3;
            *fp1 = *fp2 * 1.25f;
            
        } else {
            /* Path 2: Different computation pattern */
            vec3 = vec4 + vec1 * 0.5f;
            dvec2 = dvec1 - dvec2 * 2.0;
            
            v2 = v3 << 2 | v4;
            v7 = v8 * v9 / (v10 + 1);
            
            /* Clobber different registers */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                                       "xmm8", "xmm9", "xmm10", "xmm11");
            
            clobber_func2();
            
            asm volatile ("" ::: "memory", "r14", "r15", 
                                       "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* More vector operations */
            ivec1 = ivec1 + ivec2 * 2;
            vec1 = vec3 + vec4;
            
            f3 = f4 * 2.0f - f5;
            *dp1 = d2 + d3;
        }
        
        /* Common code with another call */
        if (i % 3 == 0) {
            /* Mix all variable types */
            v4 = v5 + v6 * (v7 >> 1);
            f4 = f1 + f2 * 0.75f;
            d4 = d1 * 1.1 + d2;
            
            vec4 = vec1 + vec2 + vec3;
            
            /* Clobber ALL caller-saved registers */
            asm volatile ("" ::: "memory", 
                "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
            
            clobber_func3();
            
            /* Force re-materialization of values */
            v1 = v1 + 1;
            f1 = f1 * 1.01f;
            d1 = d1 * 1.001;
        }
        
        /* Accumulate results to prevent elimination */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        accumulator += (long long)(f1 + f2 + f3 + f4 + f5);
        accumulator += (long long)(d1 + d2 + d3 + d4);
        
        /* Vector reduction */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                       vec2[0] + vec2[1] + vec2[2] + vec2[3];
        accumulator += (long long)vec_sum;
    }
    
    /* Final computation using all variables */
    int final_result = 
        v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        (int)(f1 + f2 + f3 + f4 + f5) +
        (int)(d1 + d2 + d3 + d4) +
        (int)(vec1[0] + vec1[1] + vec1[2] + vec1[3]) +
        (int)(vec2[0] + vec2[1] + vec2[2] + vec2[3]) +
        (int)(vec3[0] + vec3[1] + vec3[2] + vec3[3]) +
        (int)(vec4[0] + vec4[1] + vec4[2] + vec4[3]) +
        ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3] +
        (int)(dvec1[0] + dvec1[1]) +
        (int)(dvec2[0] + dvec2[1]);
    
    printf("Result: %d (accumulator: %lld)\n", final_result, accumulator);
    
    return final_result % 256;
}
