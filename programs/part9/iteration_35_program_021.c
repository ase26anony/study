#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force caller-save behavior */
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
    /* Integer variables - will compete for integer registers */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    /* Floating point variables - will compete for FP/vector registers */
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    
    /* Pointer variables - additional integer register pressure */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    
    /* Vector variables - SSE/AVX register pressure */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Create data dependencies between variables */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile int result = 0;
    
    /* Loop with conditional control flow to create complex basic blocks */
    for (int i = 0; i < iterations; i++) {
        /* Complex pre-call computations keeping many variables live */
        if (i % 2 == 0) {
            /* Branch 1: Integer-heavy computations */
            v1 = v2 + v3 * v4 - v5;
            v6 = v7 ^ v8 | v9 & v10;
            *p1 = v1 + v6;
            
            /* Vector operations mixed with scalar */
            vec1 = vec1 + vec2;
            ivec1 = ivec1 + ivec2;
            f1 = vec1[0] + vec1[1];
            
            /* Force specific register clobbering before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* More clobbering after call */
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9");
            
            /* Post-call computations using same variables */
            v2 = v1 * 2 - v3;
            vec2 = vec1 * 2.0f;
            f2 = f1 * 3.14f;
        } else {
            /* Branch 2: FP-heavy computations */
            f3 = f1 * f2 + f4;
            d1 = d2 * d3 / 1.414;
            dvec1 = dvec1 + dvec2;
            
            /* Mix with integer operations */
            v3 = (int)(f3 * 100) + v4;
            *p2 = v3 + v5;
            
            /* Clobber FP/vector registers specifically */
            asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Different external call */
            clobber_func2();
            
            /* Clobber more registers */
            asm volatile ("" ::: "memory", "xmm8", "xmm9", "xmm10", "xmm11");
            
            /* Continue computations */
            dvec2 = dvec1 * 1.5;
            f4 = (float)d1 + f3;
            v4 = v3 + (int)f4;
        }
        
        /* Common code with another call site */
        if (i % 3 == 0) {
            /* Mixed operations */
            vec1 = vec1 + (v4si){1, 1, 1, 1};
            dvec1 = dvec1 + (v2df){0.5, 0.5};
            
            /* Force spill/restore around call */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            
            clobber_func3();
            
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
            
            /* Use results */
            v5 = vec1[0] + dvec1[0];
        }
        
        /* Additional nested condition for more BB complexity */
        if (v1 > v2 && f1 < f2) {
            /* Complex expression with many live vars */
            result += v1 + v2 + v3 + v4 + v5 + v6 + 
                     (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                     vec1[0] + vec1[1] + dvec1[0];
            
            /* Another call with different clobbers */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13", "r14", "r15");
            
            clobber_func4();
            
            asm volatile ("" ::: "memory", "xmm12", "xmm13", "xmm14", "xmm15");
        }
        
        /* Loop-carried dependencies to prevent optimization */
        v1 = v1 + 1;
        v2 = v2 + 2;
        f1 = f1 + 0.1f;
        f2 = f2 + 0.2f;
        vec1 = vec1 + (v4sf){0.1f, 0.1f, 0.1f, 0.1f};
        dvec1 = dvec1 + (v2df){0.01, 0.01};
    }
    
    /* Final aggregation to prevent dead code elimination */
    int final_result = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                      (int)d1 + (int)d2 + (int)d3 +
                      vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                      dvec1[0] + dvec1[1] +
                      ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
