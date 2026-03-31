#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force calls */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE variables to prevent optimization */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f, vf5 = 5.5f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44, vd5 = 5.55;
    
    /* Non-volatile variables for computations */
    int i1 = 100, i2 = 200, i3 = 300, i4 = 400, i5 = 500;
    int i6 = 600, i7 = 700, i8 = 800, i9 = 900, i10 = 1000;
    float f1 = 10.1f, f2 = 20.2f, f3 = 30.3f, f4 = 40.4f, f5 = 50.5f;
    double d1 = 100.1, d2 = 200.2, d3 = 300.3, d4 = 400.4, d5 = 500.5;
    
    /* Pointer variables */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    float *fp1 = &f1, *fp2 = &f2;
    double *dp1 = &d1, *dp2 = &d2;
    
    /* Vector variables - these use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Mixed computations to create data dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pre-call computations - keep many variables live */
        i1 = i2 + i3 * iter;
        i4 = i5 ^ i6;
        i7 = i8 & i9;
        i10 = i1 | i2;
        
        f1 = f2 * f3 + (float)iter;
        f4 = f5 / f2 - f3;
        f5 = f1 * f4;
        
        d1 = d2 + d3 * iter;
        d4 = d5 - d2;
        d5 = d1 * d4;
        
        /* Vector operations - use SSE/AVX registers */
        vec1 = vec1 + vec2 * (float)(iter + 1);
        vec3 = vec1 - vec2;
        dvec1 = dvec1 + dvec2 * (double)iter;
        ivec1 = ivec1 + ivec2 * iter;
        
        /* Pointer chasing to force memory accesses */
        *p1 = *p2 + *p3;
        *fp1 = *fp2 * 2.0f;
        *dp1 = *dp2 / 2.0;
        
        /* ASM to clobber specific integer registers */
        asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", 
                      "rsi", "rdi", "r8", "r9", "r10", "r11");
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber specific floating point/vector registers */
        asm volatile ("" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9",
                      "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* More computations between calls */
        i2 = i3 + i4 * (iter + 2);
        i5 = i6 - i7;
        i8 = i9 * i10;
        
        f2 = f3 + f4 * (float)(iter * 2);
        f3 = f4 - f5;
        
        d2 = d3 + d4 * (iter + 1);
        d3 = d4 - d5;
        
        vec2 = vec3 * 2.0f + vec1;
        dvec2 = dvec1 * 3.0;
        ivec2 = ivec1 << 1;
        
        /* Second ASM with different clobber list */
        asm volatile ("" : : : "memory", "r12", "r13", "r14", "r15",
                      "xmm16", "xmm17", "xmm18", "xmm19");
        
        /* Second external call */
        clobber_func2();
        
        /* Post-call computations - variables must be restored */
        i3 = i4 ^ i5;
        i6 = i7 & i8;
        i9 = i10 | i1;
        
        f4 = f5 * f1 + f2;
        f5 = f1 / f3 - f4;
        
        d4 = d5 + d1 * d2;
        d5 = d2 - d3 / d4;
        
        vec3 = vec1 + vec2 * 0.5f;
        dvec1 = dvec2 - dvec1;
        ivec1 = ivec2 | ivec1;
        
        /* Third ASM and call */
        asm volatile ("" : : : "memory", "ymm0", "ymm1", "ymm2", "ymm3");
        
        clobber_func3();
        
        /* Final computations in the iteration */
        vi1 = i1 + i2;
        vi2 = i3 - i4;
        vf1 = f1 * f2;
        vf2 = f3 / f4;
        vd1 = d1 + d2;
        vd2 = d3 - d4;
        
        /* Mix vector and scalar results */
        float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3];
        double dvec_sum = dvec1[0] + dvec1[1];
        int ivec_sum = ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        
        /* Use volatile variables to force memory writes */
        vi3 = (int)vec_sum;
        vi4 = (int)dvec_sum;
        vi5 = ivec_sum;
    }
    
    /* Aggregate results to prevent dead code elimination */
    long long total = 0;
    total += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    total += (long long)(f1 + f2 + f3 + f4 + f5);
    total += (long long)(d1 + d2 + d3 + d4 + d5);
    total += vi1 + vi2 + vi3 + vi4 + vi5;
    total += (long long)(vf1 + vf2 + vf3 + vf4 + vf5);
    total += (long long)(vd1 + vd2 + vd3 + vd4 + vd5);
    
    printf("Result: %lld\n", total);
    return (int)(total % 1000);
}
