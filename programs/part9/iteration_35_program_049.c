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

/* Force non-inline behavior */
__attribute__((noinline)) static void use_vars(
    volatile int* restrict out,
    volatile float* restrict fout,
    volatile double* restrict dout
) {
    /* Empty function to create call site */
    asm volatile("" : : "r"(out), "r"(fout), "r"(dout) : "memory");
}

int main(int argc, char** argv) {
    /* Create massive register pressure with mixed types */
    
    /* Integer variables - all distinct to prevent CSE */
    volatile int v1 = argc * 1;
    volatile int v2 = argc * 2 + 1;
    volatile int v3 = argc * 3 + 2;
    volatile int v4 = argc * 4 + 3;
    volatile int v5 = argc * 5 + 4;
    volatile int v6 = argc * 6 + 5;
    volatile int v7 = argc * 7 + 6;
    volatile int v8 = argc * 8 + 7;
    volatile int v9 = argc * 9 + 8;
    volatile int v10 = argc * 10 + 9;
    
    /* Floating point variables */
    volatile float f1 = v1 * 1.1f;
    volatile float f2 = v2 * 1.2f;
    volatile float f3 = v3 * 1.3f;
    volatile float f4 = v4 * 1.4f;
    volatile float f5 = v5 * 1.5f;
    
    /* Double precision variables */
    volatile double d1 = v1 * 2.1;
    volatile double d2 = v2 * 2.2;
    volatile double d3 = v3 * 2.3;
    volatile double d4 = v4 * 2.4;
    volatile double d5 = v5 * 2.5;
    
    /* Pointer variables (create aliasing pressure) */
    volatile int* p1 = &v1;
    volatile int* p2 = &v2;
    volatile int* p3 = &v3;
    volatile float* pf1 = &f1;
    volatile float* pf2 = &f2;
    volatile double* pd1 = &d1;
    volatile double* pd2 = &d2;
    
    /* Vector variables - use all vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f5};
    v4sf vec3 = {f3, f4, f5, f1};
    v4sf vec4 = {f4, f5, f1, f2};
    
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d3};
    v2df dvec3 = {d3, d4};
    v2df dvec4 = {d4, d5};
    
    v4si ivec1 = {v1, v2, v3, v4};
    v4si ivec2 = {v2, v3, v4, v5};
    v4si ivec3 = {v3, v4, v5, v6};
    v4si ivec4 = {v4, v5, v6, v7};
    
    /* Loop to create control flow complexity */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile int sum_int = 0;
    volatile float sum_float = 0.0f;
    volatile double sum_double = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation before call - keep all vars live */
        v1 = v2 * v3 + i;
        v2 = v3 ^ v4 | i;
        v3 = v4 + v5 * i;
        v4 = v5 - v6 / (i + 1);
        v5 = v6 & v7 | (i << 2);
        
        f1 = f2 * f3 + i * 0.1f;
        f2 = f3 - f4 * i;
        f3 = f4 / (f5 + i);
        f4 = f5 * 2.0f + i;
        f5 = f1 + f2 - f3;
        
        d1 = d2 * d3 + i * 0.01;
        d2 = d3 / (d4 + i);
        d3 = d4 - d5 * i;
        d4 = d5 + d1 * i;
        d5 = d2 * 3.0 - i;
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)i;
        vec2 = vec3 - vec4 / (float)(i + 1);
        vec3 = vec4 * vec1 + (float)(i * 2);
        vec4 = vec1 / vec2 - (float)i;
        
        dvec1 = dvec2 + dvec3 * (double)i;
        dvec2 = dvec3 - dvec4 / (double)(i + 1);
        dvec3 = dvec4 * dvec1;
        dvec4 = dvec1 / dvec2;
        
        ivec1 = ivec2 + ivec3 * i;
        ivec2 = ivec3 - ivec4 / (i + 1);
        ivec3 = ivec4 & ivec1 | i;
        ivec4 = ivec1 ^ ivec2;
        
        /* Pointer chasing to create memory dependencies */
        *p1 = *p2 + *p3;
        *p2 = *p3 - *p1;
        *pf1 = *pf2 * f3;
        *pf2 = *pf1 / f4;
        *pd1 = *pd2 + d3;
        *pd2 = *pd1 - d4;
        
        /* Clobber integer registers before call */
        asm volatile("" : : : 
            "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
            "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
            "memory");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* Clobber vector registers after call */
        asm volatile("" : : : 
            "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
            "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
            "xmm12", "xmm13", "xmm14", "xmm15",
            "memory");
        
        /* More computations to keep variables live */
        v6 = v7 * v8 + v1;
        v7 = v8 ^ v9 | v2;
        v8 = v9 + v10 * v3;
        v9 = v10 - v1 / (v4 + 1);
        v10 = v1 & v2 | (v5 << 2);
        
        /* Conditional call based on computation */
        if (v1 > v2) {
            /* Clobber different registers */
            asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
            clobber_func2();
        } else {
            asm volatile("" : : : "rcx", "rdx", "xmm3", "xmm4", "xmm5", "memory");
            clobber_func3();
        }
        
        /* Complex vector operations */
        vec1 = vec1 + vec2 * vec3 - vec4;
        vec2 = vec2 - vec3 / vec4 + vec1;
        
        dvec1 = dvec1 * 2.0 + dvec2 - dvec3;
        dvec2 = dvec2 / 3.0 * dvec3 + dvec4;
        
        ivec1 = ivec1 << 1 | ivec2;
        ivec2 = ivec2 >> 2 & ivec3;
        
        /* Accumulate results */
        sum_int += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        sum_float += f1 + f2 + f3 + f4 + f5;
        sum_double += d1 + d2 + d3 + d4 + d5;
        
        /* Use vector elements */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        double dvec_sum = dvec1[0] + dvec1[1];
        int ivec_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        
        sum_float += vec_sum;
        sum_double += dvec_sum;
        sum_int += ivec_sum;
        
        /* Another asm barrier with register clobbering */
        asm volatile("" : : : 
            "rax", "rbx", "rcx", "xmm0", "xmm1", "xmm2", "xmm3",
            "memory");
    }
    
    /* Final computation and output to prevent elimination */
    volatile double final_result = 
        (double)sum_int + (double)sum_float + sum_double;
    
    /* Force use of all pointer variables */
    *p1 = sum_int;
    *pf1 = sum_float;
    *pd1 = sum_double;
    
    printf("Result: %f (argc=%d)\n", final_result, argc);
    
    /* Return value based on computation */
    return (final_result > 0.0) ? 0 : 1;
}
