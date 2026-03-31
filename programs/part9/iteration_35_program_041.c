#include <stdio.h>
#include <stdlib.h>

/* External functions to force caller-save behavior */
extern void foo(void);
extern void bar(void);
extern void baz(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    /* Force many live variables with mixed types */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    
    /* Pointer variables */
    int *p1 = &v1;
    int *p2 = &v2;
    float *fp1 = &f1;
    float *fp2 = &f2;
    double *dp1 = &d1;
    double *dp2 = &d2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Create data dependencies between variables */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; /* Bound iterations */
    
    volatile int result = 0;
    
    /* Loop to create complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations keeping variables live */
        v1 = v2 + v3 * i;
        v4 = v5 ^ v6;
        v7 = v8 << (i % 4);
        
        f1 = f2 * f3 + (float)i;
        f4 = f1 - f2;
        
        d1 = d2 / d3 + (double)i;
        d4 = d1 * d2;
        
        /* Pointer arithmetic */
        *p1 = *p2 + i;
        *fp1 = *fp2 * 1.5f;
        *dp1 = *dp2 / 2.0;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (float)(i + 1);
        dvec1 = dvec1 + dvec2;
        ivec1 = ivec1 & ivec2;
        
        /* Conditional control flow around calls */
        if (i % 2 == 0) {
            /* Clobber integer and vector registers before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                          "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* External call - forces caller-save */
            foo();
            
            /* Clobber different registers after call */
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                          "xmm4", "xmm5", "xmm6", "xmm7");
        } else {
            /* Alternative path with different register clobbering */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                          "xmm8", "xmm9", "xmm10", "xmm11");
            
            bar();
            
            asm volatile ("" ::: "memory", "r14", "r15", 
                          "xmm12", "xmm13", "xmm14", "xmm15");
        }
        
        /* More computations after call - variables must be restored */
        v2 = v1 - v4;
        v5 = v6 | v7;
        v8 = v3 >> (i % 3);
        
        f2 = f3 * f4 - (float)i;
        f3 = f1 + f2;
        
        d2 = d3 * d4 / (double)(i + 1);
        d3 = d1 - d4;
        
        /* More vector operations */
        vec2 = vec1 - vec2;
        dvec2 = dvec1 * 2.0;
        ivec2 = ivec1 | ivec2;
        
        /* Additional external call with mixed clobbering */
        if (i % 3 == 1) {
            asm volatile ("" ::: "memory", 
                          "rax", "rbx", "xmm0", "xmm1", "xmm2");
            baz();
            asm volatile ("" ::: "memory",
                          "rcx", "rdx", "xmm3", "xmm4", "xmm5");
        }
        
        /* Aggregate results */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        result += (int)(f1 + f2 + f3 + f4);
        result += (int)(d1 + d2 + d3 + d4);
        
        /* Use vector elements */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        result += (int)vec_sum;
    }
    
    /* Final computation and output to prevent elimination */
    int final_result = result + (int)(vec1[0] + dvec1[0]) + *p1;
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
