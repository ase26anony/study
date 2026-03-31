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
    volatile int v3 = argc + 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc + 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc + 7;
    volatile int v8 = argc * 8;
    volatile int v9 = argc + 9;
    volatile int v10 = argc * 10;
    
    /* Floating point variables - will compete for FP/vector registers */
    volatile float f1 = v1 * 1.1f;
    volatile float f2 = v2 * 2.2f;
    volatile float f3 = v3 * 3.3f;
    volatile float f4 = v4 * 4.4f;
    volatile double d1 = v5 * 1.11;
    volatile double d2 = v6 * 2.22;
    volatile double d3 = v7 * 3.33;
    volatile double d4 = v8 * 4.44;
    
    /* Vector variables - more pressure on vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f1};
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d3, d4};
    v4si ivec1 = {v1, v2, v3, v4};
    v4si ivec2 = {v5, v6, v7, v8};
    
    /* Pointer variables - more general purpose register pressure */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *p3 = &f1;
    volatile float *p4 = &f2;
    volatile double *p5 = &d1;
    volatile double *p6 = &d2;
    
    /* Additional variables to ensure everything stays live */
    volatile long l1 = (long)p1 + v1;
    volatile long l2 = (long)p2 + v2;
    volatile long long ll1 = (long long)p3 + (long long)(f1 * 100);
    volatile long long ll2 = (long long)p4 + (long long)(f2 * 100);
    
    /* Loop to create control flow complexity */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; /* Bound iterations */
    
    volatile float result_f = 0.0f;
    volatile double result_d = 0.0;
    volatile int result_i = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation before call to keep variables live */
        vec1 = vec1 + vec2 * (float)i;
        vec2 = vec2 - vec1 * 0.5f;
        
        dvec1 = dvec1 + dvec2 * (double)i;
        dvec2 = dvec2 - dvec1 * 0.25;
        
        ivec1 = ivec1 + ivec2 * i;
        ivec2 = ivec2 - ivec1 / (i + 1);
        
        /* Mix scalar operations */
        v1 = v1 + v2 * i;
        v2 = v2 - v1 / (i + 1);
        v3 = v3 * v4 + i;
        v4 = v4 / (v3 + 1) + i;
        
        f1 = f1 + f2 * i;
        f2 = f2 - f1 * 0.5f;
        d1 = d1 + d2 * i;
        d2 = d2 - d1 * 0.25;
        
        /* Force register clobbering before call */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* More clobbering after call */
        asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Conditional branch to create basic block complexity */
        if (i % 2 == 0) {
            /* More computations keeping variables live */
            v5 = v5 + v6 * i;
            v6 = v6 - v5 / (i + 2);
            f3 = f3 + f4 * i * 0.3f;
            f4 = f4 - f3 * 0.7f;
            
            /* Another clobber and call */
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            clobber_func2();
            
            asm volatile ("" ::: "memory", "r14", "r15", 
                                         "xmm12", "xmm13", "xmm14", "xmm15");
        } else {
            /* Alternative path with different computations */
            v7 = v7 + v8 * i;
            v8 = v8 - v7 / (i + 3);
            d3 = d3 + d4 * i * 0.33;
            d4 = d4 - d3 * 0.67;
            
            /* Different clobber set */
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1",
                                         "xmm2", "xmm3", "xmm4", "xmm5");
            
            clobber_func3();
            
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm6", "xmm7",
                                         "xmm8", "xmm9", "xmm10", "xmm11");
        }
        
        /* Post-call computations to ensure variables remain live */
        *p1 = *p1 + v1;
        *p2 = *p2 + v2;
        *p3 = *p3 + f1;
        *p4 = *p4 + f2;
        *p5 = *p5 + d1;
        *p6 = *p6 + d2;
        
        l1 = l1 + (long)(*p1);
        l2 = l2 + (long)(*p2);
        ll1 = ll1 + (long long)(*p3 * 100);
        ll2 = ll2 + (long long)(*p4 * 100);
        
        /* Aggregate results */
        result_f += f1 + f2 + f3 + f4;
        result_d += d1 + d2 + d3 + d4;
        result_i += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        
        /* Extract and sum vector elements */
        float vec1_elems[4];
        __builtin_memcpy(vec1_elems, &vec1, sizeof(vec1));
        for (int j = 0; j < 4; j++) {
            result_f += vec1_elems[j];
        }
        
        double dvec1_elems[2];
        __builtin_memcpy(dvec1_elems, &dvec1, sizeof(dvec1));
        for (int j = 0; j < 2; j++) {
            result_d += dvec1_elems[j];
        }
        
        int ivec1_elems[4];
        __builtin_memcpy(ivec1_elems, &ivec1, sizeof(ivec1));
        for (int j = 0; j < 4; j++) {
            result_i += ivec1_elems[j];
        }
    }
    
    /* Final computation and output to prevent optimization */
    double final_result = (double)result_f + result_d + (double)result_i;
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
