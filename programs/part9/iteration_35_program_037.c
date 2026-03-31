#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions that won't be inlined */
extern void clobber_foo(void);
extern void clobber_bar(void);
extern void clobber_baz(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force register spilling with many live variables */
int main(int argc, char **argv) {
    /* Integer variables - will use general purpose registers */
    volatile int a1 = argc + 1;
    volatile int a2 = argc * 2;
    volatile int a3 = argc + 3;
    volatile int a4 = argc * 4;
    volatile int a5 = argc + 5;
    volatile int a6 = argc * 6;
    volatile int a7 = argc + 7;
    volatile int a8 = argc * 8;
    volatile int a9 = argc + 9;
    volatile int a10 = argc * 10;
    
    /* Floating point variables - will use SSE/AVX registers */
    volatile float f1 = argc * 1.1f;
    volatile float f2 = argc * 2.2f;
    volatile float f3 = argc * 3.3f;
    volatile float f4 = argc * 4.4f;
    volatile double d1 = argc * 1.11;
    volatile double d2 = argc * 2.22;
    volatile double d3 = argc * 3.33;
    
    /* Pointer variables - more register pressure */
    volatile char *p1 = (char*)&a1;
    volatile char *p2 = (char*)&a2;
    volatile int *p3 = &a3;
    volatile float *p4 = &f1;
    
    /* Vector variables - use SIMD registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Loop to create complex control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex computation before call - keep many variables live */
        a1 = a2 + a3 * i;
        a4 = a5 ^ a6;
        a7 = a8 | a9;
        a10 = a1 + a4;
        
        f1 = f2 * f3 + (float)i;
        f4 = f1 / f2;
        d1 = d2 + d3 * i;
        
        vec1 = vec1 + vec2 * (float)i;
        vec2 = vec1 * 1.5f;
        dvec1 = dvec1 + dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Force register clobbering before call */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* External call - forces caller-save */
        clobber_foo();
        
        /* More clobbering after call */
        asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Conditional branch with different clobber sets */
        if (i % 2 == 0) {
            /* Even iteration - different computation path */
            a2 = a3 - a4;
            a5 = a6 & a7;
            f2 = f3 * 2.0f;
            d2 = d1 - d3;
            
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            clobber_bar();
            
            asm volatile ("" ::: "memory", "r14", "r15", 
                                         "xmm12", "xmm13", "xmm14", "xmm15");
            
            vec1 = vec2 - vec1;
            dvec1 = dvec2 * 2.0;
        } else {
            /* Odd iteration - alternative path */
            a3 = a4 + a5;
            a6 = a7 ^ a8;
            f3 = f4 / 2.0f;
            d3 = d1 + d2;
            
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx",
                                         "xmm0", "xmm1", "xmm2", "xmm3");
            
            clobber_baz();
            
            asm volatile ("" ::: "memory", "rsi", "rdi", 
                                         "xmm4", "xmm5", "xmm6", "xmm7");
            
            vec2 = vec1 + vec2;
            dvec2 = dvec1 / 2.0;
        }
        
        /* More computations to keep variables live */
        ivec2 = ivec1 * i;
        sum += a1 + a2 + a3 + a4 + (int)f1 + (int)d1;
        
        /* Additional nested condition for more basic block complexity */
        if (sum > 1000) {
            asm volatile ("" ::: "memory", "rax", "rbx", 
                                         "xmm0", "xmm1", "xmm2");
            clobber_foo();
            asm volatile ("" ::: "memory", "rcx", "rdx");
        }
    }
    
    /* Final aggregation to prevent dead code elimination */
    int final_sum = sum + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10
                  + (int)f1 + (int)f2 + (int)f3 + (int)f4
                  + (int)d1 + (int)d2 + (int)d3
                  + vec1[0] + vec2[1] + dvec1[0] + ivec1[2];
    
    printf("Result: %d\n", final_sum);
    
    /* Use pointers to prevent optimization */
    *p3 = final_sum;
    *p4 = (float)final_sum;
    
    return final_sum > 0 ? 0 : 1;
}
