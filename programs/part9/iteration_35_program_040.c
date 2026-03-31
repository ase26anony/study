#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force register pressure with many live variables */
int main(int argc, char *argv[]) {
    /* Integer variables - will compete for integer registers */
    volatile int v1 = argc * 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc * 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc * 7;
    volatile int v8 = argc * 8;
    
    /* Floating point variables - will compete for FP registers */
    volatile float f1 = v1 * 1.1f;
    volatile float f2 = v2 * 1.2f;
    volatile float f3 = v3 * 1.3f;
    volatile float f4 = v4 * 1.4f;
    volatile double d1 = v5 * 1.5;
    volatile double d2 = v6 * 1.6;
    volatile double d3 = v7 * 1.7;
    volatile double d4 = v8 * 1.8;
    
    /* Vector variables - will compete for vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f1};
    v2df vec3 = {d1, d2};
    v2df vec4 = {d3, d4};
    v4si vec5 = {v1, v2, v3, v4};
    v4si vec6 = {v5, v6, v7, v8};
    
    /* Pointer variables - more register pressure */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *p3 = &f1;
    volatile float *p4 = &f2;
    volatile double *p5 = &d1;
    volatile double *p6 = &d2;
    
    /* Additional variables to ensure spilling */
    volatile long l1 = v1 * 100L;
    volatile long l2 = v2 * 200L;
    volatile short s1 = v3;
    volatile short s2 = v4;
    volatile char c1 = v5;
    volatile char c2 = v6;
    
    /* Loop to create complex control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; /* Bound iterations */
    
    volatile float result_f = 0.0f;
    volatile double result_d = 0.0;
    volatile int result_i = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional to create branching */
        if (i % 2 == 0) {
            /* Branch 1: Heavy integer computation */
            v1 = v2 + v3 * i;
            v2 = v4 - v5 / (i + 1);
            v3 = v6 | v7 & v8;
            v4 = v1 ^ v2 ^ v3;
            
            /* Clobber integer registers before call */
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* Clobber different registers after call */
            asm volatile ("" ::: "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
            
            /* More computation with live variables */
            v5 = v3 + v4 * 2;
            v6 = v2 - v1 / 3;
            result_i += v1 + v2 + v3 + v4 + v5 + v6;
        } else {
            /* Branch 2: Heavy floating point computation */
            f1 = f2 * 1.5f + i * 0.1f;
            f2 = f3 / 1.3f - i * 0.2f;
            f3 = f4 + f1 * 2.0f;
            f4 = f2 - f3 * 0.5f;
            d1 = d2 * 1.7 + i * 0.3;
            d2 = d3 / 1.9 - i * 0.4;
            
            /* Clobber floating point registers */
            asm volatile ("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Another external call */
            clobber_func2();
            
            /* Clobber more registers */
            asm volatile ("" ::: "memory", "xmm8", "xmm9", "xmm10", "xmm11",
                                       "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Continue computation */
            d3 = d4 + d1 * 3.0;
            d4 = d2 - d3 * 0.25;
            result_f += f1 + f2 + f3 + f4;
            result_d += d1 + d2 + d3 + d4;
        }
        
        /* Vector operations in both branches */
        vec1 = vec1 + vec2 * (i + 1);
        vec2 = vec2 - vec1 * 0.5f;
        vec3 = vec3 + vec4;
        vec4 = vec4 * 2.0 - vec3;
        vec5 = vec5 | vec6;
        vec6 = vec6 & vec5;
        
        /* Pointer arithmetic to keep pointers live */
        *p1 += i;
        *p2 -= i;
        *p3 *= 1.0f + i * 0.01f;
        *p4 /= 1.0f + i * 0.01f;
        *p5 += i * 0.1;
        *p6 -= i * 0.1;
        
        /* Mix in other variables */
        l1 = l2 * 3 + i;
        l2 = l1 / 2 - i;
        s1 = (s1 + i) & 0xFF;
        s2 = (s2 - i) & 0xFF;
        c1 = (c1 ^ i) & 0x7F;
        c2 = (c2 | i) & 0x7F;
        
        /* Third external call with mixed clobbering */
        if (i % 3 == 0) {
            asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
            clobber_func3();
            asm volatile ("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4", "xmm5");
        }
        
        /* Final computation using all variable types */
        result_i += vec5[0] + vec5[1] + vec5[2] + vec5[3];
        result_f += vec1[0] + vec1[1] + vec1[2] + vec1[3];
        result_d += vec3[0] + vec3[1];
    }
    
    /* Aggregate results to prevent dead code elimination */
    double final_result = result_i + result_f + result_d + 
                         l1 + l2 + s1 + s2 + c1 + c2 +
                         vec1[0] + vec2[1] + vec3[0] + vec4[1] + vec5[2] + vec6[3];
    
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
