#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force calls */
extern void external_func1(void);
extern void external_func2(void);
extern void external_func3(void);

/* Vector types for SSE/AVX pressure */
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
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Mix computations to create dependencies */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pre-call computations - keep many variables live */
        i1 = i2 + i3 * iter;
        i4 = i5 ^ i6;
        i7 = i8 - i9;
        i10 = i1 * i4;
        
        f1 = f2 * f3 + (float)iter;
        f4 = f5 / (f1 + 1.0f);
        f5 = f4 * 2.0f - f3;
        
        d1 = d2 + d3 * (double)iter;
        d4 = d5 * 0.5 - d1;
        d5 = d4 + d2 / d3;
        
        /* Vector operations */
        vec1 = vec2 + vec3 * (float)(iter + 1);
        vec2 = vec1 - vec3;
        vec3 = vec1 * vec2;
        
        dvec1 = dvec2 * (double)(iter + 2);
        dvec2 = dvec1 + dvec2;
        
        ivec1 = ivec2 << (iter % 4);
        ivec2 = ivec1 + ivec2;
        
        /* Pointer arithmetic */
        p1 = p2 + iter;
        p3 = p1 - iter;
        *p1 = *p2 + *p3;
        
        fp1 = fp2 + iter;
        *fp1 = *fp2 * (float)iter;
        
        dp1 = dp2;
        *dp1 = *dp2 / (double)(iter + 1);
        
        /* CLOBBER integer registers before call */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* CLOBBER vector registers */
        asm volatile("" : : : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                     "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Force memory clobber */
        asm volatile("" : : : "memory");
        
        /* External function call - cannot be inlined */
        external_func1();
        
        /* Different clobber set after call */
        asm volatile("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        
        /* More computations using live variables */
        i2 = i1 + i10;
        i3 = i4 * i7;
        i5 = i6 ^ i8;
        i9 = i2 - i3;
        
        f2 = f1 * f4;
        f3 = f5 + f2;
        f4 = f3 / f1;
        
        d2 = d1 * d4;
        d3 = d5 - d2;
        d4 = d3 * 0.75;
        
        vec2 = vec1 + vec3;
        vec1 = vec2 * 0.5f;
        
        dvec2 = dvec1 * 2.0;
        
        ivec2 = ivec1 | ivec2;
        
        /* Second clobber and call */
        asm volatile("" : : : "rcx", "rdx", "xmm4", "xmm5", "xmm6", "xmm7", "memory");
        
        external_func2();
        
        /* Third clobber with different registers */
        asm volatile("" : : : "rsi", "rdi", "xmm8", "xmm9", "xmm10", "xmm11", "memory");
        
        /* More computations */
        *p2 = *p1 + iter;
        *p3 = *p2 * 2;
        
        *fp2 = *fp1 + (float)iter;
        
        *dp2 = *dp1 * (double)(iter + 1);
        
        external_func3();
        
        /* Final clobber to force save/restore */
        asm volatile("" : : : "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3",
                     "xmm12", "xmm13", "xmm14", "xmm15", "memory");
    }
    
    /* Aggregate results to prevent dead code elimination */
    int int_sum = i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
    float float_sum = f1 + f2 + f3 + f4 + f5;
    double double_sum = d1 + d2 + d3 + d4 + d5;
    
    /* Vector reduction */
    float vec_sum = vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                   vec2[0] + vec2[1] + vec2[2] + vec2[3] +
                   vec3[0] + vec3[1] + vec3[2] + vec3[3];
    
    /* Use volatile variables */
    int_sum += vi1 + vi2 + vi3 + vi4 + vi5;
    float_sum += vf1 + vf2 + vf3 + vf4 + vf5;
    double_sum += vd1 + vd2 + vd3 + vd4 + vd5;
    
    printf("Results: int_sum=%d, float_sum=%f, double_sum=%lf, vec_sum=%f\n",
           int_sum, float_sum, double_sum, vec_sum);
    
    return int_sum > 0 ? 0 : 1;
}
