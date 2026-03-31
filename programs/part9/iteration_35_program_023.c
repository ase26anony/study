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
    float f1 = 100.1f, f2 = 200.2f, f3 = 300.3f, f4 = 400.4f, f5 = 500.5f;
    double d1 = 100.11, d2 = 200.22, d3 = 300.33, d4 = 400.44, d5 = 500.55;
    
    /* Pointer variables */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3;
    float *fp1 = &f1, *fp2 = &f2, *fp3 = &f3;
    double *dp1 = &d1, *dp2 = &d2, *dp3 = &d3;
    
    /* Vector variables - these use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {10, 20, 30, 40};
    v4si ivec2 = {50, 60, 70, 80};
    
    /* Accumulator to prevent dead code elimination */
    double total = 0.0;
    
    /* Loop to create complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* Pre-call computations - keep many variables live */
        i1 = vi1 + iter;
        i2 = vi2 * i1;
        i3 = vi3 + i2;
        i4 = vi4 - i3;
        i5 = vi5 ^ i4;
        
        f1 = vf1 * (float)iter;
        f2 = vf2 + f1;
        f3 = vf3 - f2;
        f4 = vf4 / (f3 + 0.001f);
        f5 = vf5 * f4;
        
        d1 = vd1 + (double)iter;
        d2 = vd2 * d1;
        d3 = vd3 + d2;
        d4 = vd4 - d3;
        d5 = vd5 / (d4 + 0.001);
        
        /* Vector operations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec1 - vec2;
        
        dvec1 = dvec1 + dvec2;
        dvec2 = dvec1 * dvec2;
        
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec1 - ivec2;
        
        /* Pointer arithmetic */
        *p1 = i1 + *p2;
        *p2 = i2 + *p3;
        *fp1 = f1 + *fp2;
        *fp2 = f2 + *fp3;
        *dp1 = d1 + *dp2;
        *dp2 = d2 + *dp3;
        
        /* CLOBBER integer registers before call */
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Function call that forces caller-save */
        clobber_func1();
        
        /* CLOBBER vector registers after call */
        asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "xmm8", "xmm9", "xmm10", "xmm11",
                      "xmm12", "xmm13", "xmm14", "xmm15", "memory");
        
        /* More computations to keep variables live across calls */
        i1 = i1 + i5;
        i2 = i2 * i4;
        i3 = i3 ^ i2;
        
        f1 = f1 + f5;
        f2 = f2 * f4;
        f3 = f3 / f2;
        
        d1 = d1 + d5;
        d2 = d2 * d4;
        d3 = d3 - d2;
        
        /* More vector ops */
        vec1 = vec3 + vec2;
        vec2 = vec1 * vec3;
        
        /* CLOBBER different registers */
        asm volatile ("" : : : "rax", "rbx", "xmm0", "xmm1", "xmm2", "memory");
        
        /* Another function call */
        clobber_func2();
        
        /* Mixed clobber */
        asm volatile ("" : : : "rcx", "rdx", "xmm3", "xmm4", "xmm5", "memory");
        
        /* Conditional inside loop to create more basic blocks */
        if (iter % 2 == 0) {
            /* Even iteration computations */
            i4 = i1 + i2 + i3;
            f4 = f1 + f2 + f3;
            d4 = d1 + d2 + d3;
            
            /* CLOBBER and call in conditional path */
            asm volatile ("" : : : "rsi", "rdi", "xmm6", "xmm7", "memory");
            clobber_func3();
            asm volatile ("" : : : "r8", "r9", "xmm8", "xmm9", "memory");
            
            vec3 = vec1 + vec2;
            dvec1 = dvec1 + dvec2;
        } else {
            /* Odd iteration computations */
            i5 = i1 * i2 * i3;
            f5 = f1 * f2 * f3;
            d5 = d1 * d2 * d3;
            
            vec3 = vec1 * vec2;
            dvec1 = dvec1 * dvec2;
        }
        
        /* Accumulate results to prevent elimination */
        total += (double)i1 + (double)i2 + (double)i3 + (double)i4 + (double)i5 +
                 (double)f1 + (double)f2 + (double)f3 + (double)f4 + (double)f5 +
                 d1 + d2 + d3 + d4 + d5 +
                 (double)vec1[0] + (double)vec2[1] + (double)vec3[2] +
                 dvec1[0] + dvec2[1];
    }
    
    /* Final computation and output */
    double final_result = total / (iterations + 1.0);
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
