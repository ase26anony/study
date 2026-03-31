#include <stdio.h>
#include <stdlib.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force specific register clobbering */
#define CLOBBER_INT_REGS asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi")
#define CLOBBER_FLOAT_REGS asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5")
#define CLOBBER_ALL_REGS asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
                                      "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7")

int main(int argc, char **argv) {
    /* Force conditional control flow based on command line */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables with mixed types to maximize register pressure */
    
    /* Integer variables - force live across calls */
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
    
    /* Floating point variables */
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    
    /* Double precision variables */
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *p3 = &f1;
    volatile double *p4 = &d1;
    
    /* Vector variables - use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.11, 2.22};
    v2df dvec2 = {3.33, 4.44};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Accumulator to prevent optimization */
    volatile double total = 0.0;
    
    /* Complex loop with conditional control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations - create data dependencies */
        v1 = v2 + v3 * i;
        v4 = v5 ^ v6;
        v7 = v8 | v9;
        v10 = v1 + v4 - v7;
        
        f1 = f2 * f3 + (float)i;
        f4 = f5 / f2;
        f3 = f1 + f4;
        
        d1 = d2 * d3 - (double)v1;
        d4 = d1 / d2 + d3;
        
        /* Vector operations - use SSE/AVX registers */
        vec1 = vec1 + vec2 * (v4sf){0.5f, 0.5f, 0.5f, 0.5f};
        vec3 = vec1 - vec2;
        dvec1 = dvec1 + dvec2 * (v2df){0.25, 0.25};
        ivec1 = ivec1 + ivec2 * i;
        
        /* Pointer arithmetic */
        *p1 = *p2 + i;
        *p3 = *p3 + f2;
        *p4 = *p4 + d2;
        
        /* Clobber integer registers before first call */
        CLOBBER_INT_REGS;
        
        /* First external call - forces caller-save */
        clobber_func1();
        
        /* Intermediate computations to keep variables live */
        v2 = v3 + v4 * 2;
        f2 = f3 * 1.5f;
        d2 = d3 * 2.0;
        vec2 = vec3 + (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
        
        /* Clobber float registers */
        CLOBBER_FLOAT_REGS;
        
        /* Second external call */
        clobber_func2();
        
        /* More computations */
        v5 = v6 - v7 / 3;
        f5 = f4 * f1;
        d3 = d4 - d1;
        dvec2 = dvec1 * (v2df){0.5, 0.5};
        
        /* Conditional branch inside loop - creates complex CFG */
        if (i % 2 == 0) {
            /* Clobber all registers */
            CLOBBER_ALL_REGS;
            
            /* Third external call in conditional path */
            clobber_func3();
            
            /* Different computations in this path */
            v8 = v9 * v10;
            vec3 = vec1 * vec2;
        } else {
            /* Alternative computations */
            v8 = v9 + v10;
            vec3 = vec1 + vec2;
        }
        
        /* Post-call computations - ensure all variables remain live */
        v9 = v8 + v1;
        v6 = v5 ^ v4;
        f3 = f2 + f5;
        f4 = f1 * f3;
        d4 = d1 + d2 + d3;
        vec1 = vec2 + vec3;
        dvec1 = dvec1 + dvec2;
        ivec2 = ivec1 + (v4si){i, i, i, i};
        
        /* Accumulate results to prevent optimization */
        total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
               + f1 + f2 + f3 + f4 + f5
               + d1 + d2 + d3 + d4
               + vec1[0] + vec1[1] + vec1[2] + vec1[3]
               + dvec1[0] + dvec1[1];
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %f\n", total);
    
    /* Additional volatile store to force register spills */
    volatile int final_check = v1 + v2 + v3;
    (void)final_check;
    
    return 0;
}
