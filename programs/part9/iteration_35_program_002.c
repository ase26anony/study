#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that won't be inlined */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char *argv[]) {
    /* Force many live variables with mixed types */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    
    /* Vector variables - use many to pressure SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vec4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = {5.0, 6.0};
    
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional integer variables */
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    /* Use command line argument to control loop iterations */
    int iterations = 3;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    /* Complex accumulator to prevent optimization */
    volatile double complex_acc = 0.0;
    volatile v4sf vec_acc = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* Main loop with conditional control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        if (i % 2 == 0) {
            /* Even iteration: integer and float operations */
            v1 = v2 + v3 * v4;
            f1 = f2 * f3 - f4;
            d1 = d2 / d3 + d4;
            
            /* Vector operations */
            vec1 = vec2 + vec3;
            vec4 = vec1 * vec2;
            
            /* Pointer arithmetic */
            *p1 = v5 + v6;
            *fp1 = f3 * 2.0f;
            
            /* Clobber integer and vector registers before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* Clobber different registers after call */
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                         "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Post-call computations */
            v2 = v1 - v3;
            f2 = f1 + f4;
            dvec1 = dvec2 + dvec3;
            
            /* More vector operations */
            vec2 = vec3 - vec4;
            ivec1 = ivec1 + ivec2;
        } else {
            /* Odd iteration: different operations */
            v5 = v6 * v7 - v8;
            f3 = f4 / f1 + f2;
            d2 = d3 * d4 - d1;
            
            /* Vector operations with different registers */
            vec3 = vec4 * vec1;
            dvec2 = dvec3 - dvec1;
            
            /* Clobber more registers */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            /* Different external call */
            clobber_func2();
            
            /* Clobber yet another set */
            asm volatile ("" ::: "memory", "r14", "r15", 
                         "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* More computations */
            v6 = v5 + v9;
            f4 = f3 * f2;
            vec4 = vec1 + vec2;
        }
        
        /* Additional conditional branch inside loop */
        if (v1 > 100) {
            /* Rare path with another call */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            clobber_func3();
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
        }
        
        /* Accumulate results to prevent dead code elimination */
        complex_acc += v1 + v2 + v3 + v4 + v5 + v6;
        complex_acc += f1 + f2 + f3 + f4;
        complex_acc += d1 + d2 + d3 + d4;
        
        /* Vector accumulation */
        vec_acc = vec_acc + vec1 + vec2 + vec3 + vec4;
        
        /* Mix scalar and vector operations */
        v7 = v8 + v9 * v10;
        ivec2 = ivec1 + ivec2;
        dvec3 = dvec1 * 2.0;
    }
    
    /* Final computation and output to prevent optimization */
    double final_result = complex_acc;
    final_result += vec_acc[0] + vec_acc[1] + vec_acc[2] + vec_acc[3];
    final_result += dvec1[0] + dvec1[1];
    final_result += dvec2[0] + dvec2[1];
    final_result += dvec3[0] + dvec3[1];
    
    printf("Result: %f\n", final_result);
    
    /* Use all variables one more time */
    volatile int dummy = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    (void)dummy;
    
    return (int)final_result % 256;
}
