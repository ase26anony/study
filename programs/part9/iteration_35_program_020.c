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
    
    /* 
     * Declare MANY local variables of mixed types to maximize register pressure
     * Mark key ones as volatile to prevent optimization
     */
    
    /* Integer variables */
    volatile int v1 = 1;
    int v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    long v6 = 6, v7 = 7, v8 = 8, v9 = 9;
    unsigned int v10 = 10, v11 = 11, v12 = 12;
    
    /* Floating point variables */
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44;
    volatile double d5 = 5.55;
    
    /* Pointer variables */
    int *p1 = &v1;
    float *p2 = &f1;
    double *p3 = &d1;
    volatile int *vp = &v2;
    
    /* Vector variables - these use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df vec3 = {1.0, 2.0};
    v2df vec4 = {3.0, 4.0};
    v4si vec5 = {1, 2, 3, 4};
    v4si vec6 = {5, 6, 7, 8};
    volatile v4sf vvec = {9.0f, 10.0f, 11.0f, 12.0f};
    
    /* Additional variables for more pressure */
    char c1 = 'a', c2 = 'b', c3 = 'c';
    short s1 = 100, s2 = 200, s3 = 300;
    volatile short vs = 400;
    
    /* Loop to create complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Phase 1: Heavy computation before call - all variables live */
        v1 = v2 + v3 * v4 - v5;
        f1 = f2 * f3 + (float)v6;
        d1 = d2 / d3 * d4 - d5;
        
        /* Vector operations - use SSE/AVX registers */
        vec1 = vec1 + vec2;
        vec3 = vec3 * vec4;
        vec5 = vec5 & vec6;
        
        /* Pointer arithmetic keeping pointers live */
        *p1 = *p1 + v7;
        *p2 = *p2 * 1.1f;
        *p3 = *p3 + 0.5;
        
        /* Mix in character/short operations */
        c1 = c2 + c3 - 'a';
        s1 = s2 + s3 + vs;
        
        /* 
         * CRITICAL: Insert asm that clobbers specific registers
         * This forces the compiler to save/restore around the call
         */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* External function call - cannot be inlined */
        clobber_func1();
        
        /* Another asm clobber with DIFFERENT registers */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations to keep variables live across calls */
        v2 = v1 * v3 + v4;
        f2 = f1 / f3 * 2.0f;
        d2 = d1 + d3 - d4;
        
        /* More vector ops */
        vec2 = vec1 * vvec;
        vec4 = vec3 + vec4;
        vec6 = vec5 | vec6;
        
        /* Second external call with different register clobbering */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        clobber_func2();
        
        asm volatile("" ::: "memory", "r14", "r15", 
                     "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Complex conditional inside loop to create more BBs */
        if (i % 2 == 0) {
            v3 = v2 * v5 + v10;
            f3 = f2 * 3.14f;
            
            /* Third call in conditional path */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            clobber_func3();
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
            
            d3 = d2 * 2.0 + d5;
            vec1 = vec2 + vvec;
        } else {
            v4 = v3 * v2 - v11;
            d4 = d3 / 2.0 - d1;
            vec3 = vec4 * 2.0;
        }
        
        /* Final computations in loop */
        v5 = v4 + v6 + v7;
        d5 = d4 * 1.1;
        *vp = *vp + i;
        
        /* More vector operations */
        vvec = vec1 + vec2;
    }
    
    /* Aggregate results to prevent dead code elimination */
    double total = 0.0;
    total += v1 + v2 + v3 + v4 + v5;
    total += v6 + v7 + v8 + v9;
    total += v10 + v11 + v12;
    total += f1 + f2 + f3;
    total += d1 + d2 + d3 + d4 + d5;
    total += c1 + c2 + c3;
    total += s1 + s2 + s3 + vs;
    
    /* Extract and sum vector elements */
    float *vf = (float*)&vec1;
    for (int j = 0; j < 4; j++) total += vf[j];
    
    vf = (float*)&vec2;
    for (int j = 0; j < 4; j++) total += vf[j];
    
    vf = (float*)&vvec;
    for (int j = 0; j < 4; j++) total += vf[j];
    
    double *vd = (double*)&vec3;
    total += vd[0] + vd[1];
    
    vd = (double*)&vec4;
    total += vd[0] + vd[1];
    
    int *vi = (int*)&vec5;
    for (int j = 0; j < 4; j++) total += vi[j];
    
    vi = (int*)&vec6;
    for (int j = 0; j < 4; j++) total += vi[j];
    
    printf("Result: %f\n", total);
    
    return (int)total % 256;
}
