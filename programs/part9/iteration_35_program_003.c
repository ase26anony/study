#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
int main(int argc, char **argv) {
    /* Integer variables - will compete for general purpose registers */
    volatile int a1 = argc + 1;
    volatile int a2 = argc * 2;
    volatile int a3 = argc + 3;
    volatile int a4 = argc * 4;
    volatile int a5 = argc + 5;
    volatile int a6 = argc * 6;
    volatile int a7 = argc + 7;
    volatile int a8 = argc * 8;
    
    /* Floating point variables - will compete for FP registers */
    volatile float f1 = argc * 1.1f;
    volatile float f2 = argc * 2.2f;
    volatile float f3 = argc * 3.3f;
    volatile float f4 = argc * 4.4f;
    volatile double d1 = argc * 1.111;
    volatile double d2 = argc * 2.222;
    volatile double d3 = argc * 3.333;
    volatile double d4 = argc * 4.444;
    
    /* Vector variables - will use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df vec3 = {1.0, 2.0};
    v2df vec4 = {3.0, 4.0};
    v4si vec5 = {1, 2, 3, 4};
    v4si vec6 = {5, 6, 7, 8};
    
    /* Pointer variables - more register pressure */
    volatile int *p1 = &a1;
    volatile int *p2 = &a2;
    volatile float *p3 = &f1;
    volatile double *p4 = &d1;
    
    /* Additional variables to ensure spilling */
    volatile long l1 = argc * 100L;
    volatile long l2 = argc * 200L;
    volatile short s1 = argc;
    volatile short s2 = argc * 2;
    
    /* Loop to create complex control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; /* Bound iterations */
    
    volatile float result_f = 0.0f;
    volatile double result_d = 0.0;
    volatile int result_i = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation before call - keeps many variables live */
        if (i % 2 == 0) {
            /* Branch 1: Integer-heavy computation */
            a1 = a2 + a3 * i;
            a4 = a5 - a6 / (i + 1);
            a7 = a8 ^ (a1 * i);
            l1 = l2 + a4 * i;
            
            /* Clobber integer registers before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
            
            /* Function call that forces caller-save */
            clobber_func1();
            
            /* Clobber different registers after call */
            asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11", "r12", "r13");
            
            /* More computation with saved variables */
            a2 = a1 + a7;
            a3 = a4 * a6;
            result_i += a1 + a2 + a3 + a4;
        } else {
            /* Branch 2: Floating-point heavy computation */
            f1 = f2 * f3 + i * 0.5f;
            f2 = f3 / f4 - i * 0.25f;
            d1 = d2 + d3 * i;
            d2 = d4 - d1 / (i + 1);
            
            /* Vector operations */
            vec1 = vec1 + vec2 * i;
            vec3 = vec3 + vec4 * i;
            vec5 = vec5 + vec6 * i;
            
            /* Clobber floating point and vector registers */
            asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Another function call */
            clobber_func2();
            
            /* Clobber more registers */
            asm volatile ("" ::: "memory", "xmm8", "xmm9", "xmm10", "xmm11",
                                       "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Continue computation */
            f3 = f1 * f2;
            f4 = f3 / f1;
            vec2 = vec1 * 2.0f;
            vec4 = vec3 * 1.5;
            
            result_f += f1 + f2 + f3 + f4;
            result_d += d1 + d2;
        }
        
        /* Mixed computation that uses all variable types */
        if (i % 3 == 0) {
            /* Use pointers to force memory operations */
            *p1 = *p2 + i;
            *p3 = *p3 * 1.1f;
            *p4 = *p4 + 0.5;
            
            /* Clobber mixed registers */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
            
            clobber_func3();
            
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4", "xmm5");
            
            /* Cross-type computation */
            a5 = (int)(f1 * 10.0f) + a1;
            f1 = (float)a2 * 0.1f + f3;
        }
        
        /* Nested conditional for more complex CFG */
        if (argc > 2) {
            volatile int temp = argv[2][0];
            a6 = a7 + temp * i;
            
            asm volatile ("" ::: "memory", "r14", "r15", "xmm6", "xmm7");
            
            clobber_func4();
            
            asm volatile ("" ::: "memory", "rax", "xmm8", "xmm9", "xmm10");
            
            a7 = a6 * 2 - temp;
        }
        
        /* Final computation in loop to keep variables live */
        result_i += a1 + a5 + a6 + a7;
        result_f += f1 + f2;
        result_d += d1 + d2;
        
        /* Vector result accumulation */
        for (int j = 0; j < 4; j++) {
            result_f += vec1[j];
            result_i += vec5[j];
        }
        for (int j = 0; j < 2; j++) {
            result_d += vec3[j];
        }
    }
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = (double)result_i + result_f + result_d;
    
    /* Use results to create observable side effect */
    printf("Result: %f (argc=%d)\n", final_result, argc);
    
    /* Additional volatile store to force all computations */
    volatile double *output = (double*)malloc(sizeof(double));
    *output = final_result;
    printf("Stored: %f\n", *output);
    free((void*)output);
    
    return (int)final_result % 256;
}
