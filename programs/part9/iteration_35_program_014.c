#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);
extern void clobber_func4(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables of mixed types to maximize register pressure */
    
    /* Integer variables */
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
    volatile double d1 = 1.111;
    volatile double d2 = 2.222;
    volatile double d3 = 3.333;
    volatile double d4 = 4.444;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *fp1 = &f1;
    volatile float *fp2 = &f2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    
    v2df dvec1 = {1.111, 2.222};
    v2df dvec2 = {3.333, 4.444};
    
    v4si ivec1 = {100, 200, 300, 400};
    v4si ivec2 = {500, 600, 700, 800};
    
    /* Additional variables to increase pressure */
    volatile long l1 = 1000;
    volatile long l2 = 2000;
    volatile long l3 = 3000;
    volatile short s1 = 10;
    volatile short s2 = 20;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop to create complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Phase 1: Intensive computations before call */
        v1 = v2 + v3 * i;
        v4 = v5 ^ v6 | v7;
        v8 = v9 << (v10 & 3);
        
        f1 = f2 * f3 + (float)i;
        f4 = f5 / (f1 + 1.0f);
        
        d1 = d2 * d3 - d4;
        d2 = d1 / (d3 + 1.0);
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (float)v1;
        vec3 = vec1 - vec2;
        dvec1 = dvec1 * dvec2 + (double)i;
        
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec1 - ivec2;
        
        /* Pointer arithmetic */
        p1 = p1 + (v1 & 1);
        fp1 = fp1 + (v2 & 1);
        
        l1 = l2 * l3 + i;
        s1 = s2 << (i & 3);
        
        /* Force register clobbering before call */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* More clobbering after call */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm6", "xmm7", "xmm8", "xmm9", "xmm10");
        
        /* Phase 2: More computations to keep variables live */
        v2 = v3 + v4 * (i + 1);
        v5 = v6 ^ v7 | v8;
        v9 = v10 << (v1 & 3);
        
        f2 = f3 * f4 + (float)(i + 1);
        f5 = f1 / (f2 + 1.0f);
        
        d3 = d4 * d1 - d2;
        d4 = d2 / (d1 + 1.0);
        
        /* More vector operations */
        vec2 = vec2 + vec3 * (float)v2;
        vec1 = vec3 - vec2;
        dvec2 = dvec2 * dvec1 + (double)(i + 1);
        
        ivec2 = ivec2 + ivec1;
        ivec1 = ivec2 - ivec1;
        
        /* Second external call with different clobbering */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13", "r14", "r15",
                     "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        clobber_func2();
        
        /* Conditional call based on loop iteration */
        if (i % 2 == 0) {
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm2");
            clobber_func3();
        } else {
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm3", "xmm4", "xmm5");
            clobber_func4();
        }
        
        /* Final computations and result accumulation */
        total_result += (double)v1 + (double)v2 + f1 + f2 + d1 + d2;
        total_result += vec1[0] + vec2[1] + vec3[2];
        total_result += dvec1[0] + dvec2[1];
        total_result += (double)ivec1[0] + (double)ivec2[1];
        
        /* Additional mixing */
        total_result += (double)(*p1) + (double)(*fp1);
        total_result += (double)l1 + (double)s1;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", total_result);
    
    /* Additional volatile store to force all values to be computed */
    volatile double final_check = total_result;
    
    return (int)(final_check * 0.0001);
}
