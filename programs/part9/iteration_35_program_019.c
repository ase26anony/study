#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* External functions to force calls */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);
extern void clobber_func4(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force register spilling with many live variables */
int main(int argc, char *argv[]) {
    /* Integer variables - will compete for general purpose registers */
    volatile int v1 = argc * 2;
    volatile int v2 = argc + 100;
    volatile int v3 = argc * 3;
    volatile int v4 = argc - 50;
    volatile int v5 = argc * argc;
    volatile int v6 = argc + 200;
    volatile int v7 = argc * 4;
    volatile int v8 = argc + 300;
    volatile int v9 = argc * 5;
    volatile int v10 = argc + 400;
    
    /* Floating point variables - will compete for FP/SSE registers */
    volatile float f1 = argc * 1.5f;
    volatile float f2 = argc * 2.5f;
    volatile float f3 = argc * 3.5f;
    volatile float f4 = argc * 4.5f;
    volatile double d1 = argc * 1.25;
    volatile double d2 = argc * 2.25;
    volatile double d3 = argc * 3.25;
    volatile double d4 = argc * 4.25;
    
    /* Vector variables - more pressure on vector registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    volatile double *dp1 = &d1;
    volatile double *dp2 = &d2;
    
    /* Complex computation to create data dependencies */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    float fsum = 0.0f;
    double dsum = 0.0;
    int isum = 0;
    
    /* Loop with conditional control flow to create complex basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        if (i % 2 == 0) {
            v1 = v2 + v3 * v4;
            v5 = v6 - v7 / (v8 + 1);
            f1 = f2 * f3 + f4;
            d1 = d2 / d3 - d4;
            
            /* Vector operations */
            vec1 = vec1 + vec2;
            dvec1 = dvec1 * dvec2;
            ivec1 = ivec1 + ivec2;
            
            /* Pointer arithmetic */
            *p1 = *p2 + v9;
            *fp1 = *fp2 * 1.1f;
            *dp1 = *dp2 / 1.5;
        } else {
            v2 = v3 + v4 * v5;
            v6 = v7 - v8 / (v9 + 1);
            f2 = f3 * f4 + f1;
            d2 = d3 / d4 - d1;
            
            vec2 = vec2 - vec1;
            dvec2 = dvec2 / dvec1;
            ivec2 = ivec2 - ivec1;
            
            *p2 = *p1 + v10;
            *fp2 = *fp1 * 0.9f;
            *dp2 = *dp1 / 1.2;
        }
        
        /* First asm volatile with register clobbering */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* External function call - forces caller-save */
        clobber_func1();
        
        /* Second asm volatile with different clobbers */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations to keep variables live across calls */
        if (i % 3 == 0) {
            v3 = v4 + v5 * v6;
            v7 = v8 - v9 / (v10 + 1);
            f3 = f4 * f1 + f2;
            d3 = d4 / d1 - d2;
            
            vec1 = vec1 * 2.0f;
            dvec1 = dvec1 + 1.0;
            ivec1 = ivec1 * 3;
        }
        
        /* Third asm volatile */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* Another external call */
        clobber_func2();
        
        /* Fourth asm volatile */
        asm volatile("" ::: "memory", "r14", "r15", 
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Complex nested condition to create more basic blocks */
        if (argc > 2) {
            /* Additional calls in conditional path */
            clobber_func3();
            
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            
            v4 = v5 + v6 * v7;
            v8 = v9 - v10 / (v1 + 1);
            f4 = f1 * f2 + f3;
            d4 = d1 / d2 - d3;
            
            vec2 = vec2 / 1.5f;
            dvec2 = dvec2 - 0.5;
            ivec2 = ivec2 / 2;
        } else {
            /* Alternative path with different clobbers */
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
            
            clobber_func4();
        }
        
        /* Post-call computations */
        v9 = v10 + v1 * v2;
        v10 = v1 - v2 / (v3 + 1);
        
        /* Accumulate results to prevent elimination */
        fsum += f1 + f2 + f3 + f4;
        dsum += d1 + d2 + d3 + d4;
        isum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Vector accumulation */
        for (int j = 0; j < 4; j++) {
            fsum += vec1[j] + vec2[j];
            isum += ivec1[j] + ivec2[j];
        }
        for (int j = 0; j < 2; j++) {
            dsum += dvec1[j] + dvec2[j];
        }
    }
    
    /* Final computation and output to prevent dead code elimination */
    double total = fsum + dsum + isum;
    printf("Result: %f (argc=%d)\n", total, argc);
    
    return (total > 0) ? 0 : 1;
}
