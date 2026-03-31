#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
    
    /* Declare MANY local variables of mixed types to create register pressure */
    
    /* Integer variables */
    volatile int int1 = 1;
    volatile int int2 = 2;
    volatile int int3 = 3;
    volatile int int4 = 4;
    volatile int int5 = 5;
    volatile int int6 = 6;
    volatile int int7 = 7;
    volatile int int8 = 8;
    volatile int int9 = 9;
    volatile int int10 = 10;
    
    /* Floating point variables */
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile float f5 = 5.5f;
    
    /* Double precision variables */
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    volatile double d5 = 5.55;
    
    /* Pointer variables */
    volatile int *ptr1 = &int1;
    volatile int *ptr2 = &int2;
    volatile float *ptr3 = &f1;
    volatile double *ptr4 = &d1;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df vecd1 = {1.0, 2.0};
    v2df vecd2 = {3.0, 4.0};
    v4si veci1 = {1, 2, 3, 4};
    v4si veci2 = {5, 6, 7, 8};
    
    /* Additional variables for more pressure */
    volatile long long ll1 = 100;
    volatile long long ll2 = 200;
    volatile long long ll3 = 300;
    volatile long long ll4 = 400;
    
    /* Mix all types in computations to keep them live */
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations using all variables */
        int1 = int2 + int3 * i;
        int4 = int5 ^ int6;
        int7 = int8 | int9;
        int10 = int1 & int2;
        
        f1 = f2 * f3 + (float)i;
        f4 = f5 / f2;
        f3 = f1 - f4;
        
        d1 = d2 * d3 + (double)i;
        d4 = d5 / d2;
        d3 = d1 - d4;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (v4sf){1.0f, 1.0f, 1.0f, 1.0f};
        vec3 = vec1 - vec2;
        vecd1 = vecd1 + vecd2;
        
        /* Pointer arithmetic */
        *ptr1 = *ptr1 + 1;
        *ptr2 = *ptr2 - 1;
        *ptr3 = *ptr3 * 1.1f;
        *ptr4 = *ptr4 * 1.01;
        
        /* Long long operations */
        ll1 = ll2 + ll3;
        ll4 = ll1 - ll2;
        
        /* Integer vector operations */
        veci1 = veci1 + veci2;
        veci2 = veci1 - veci2;
        
        /* 
         * CRITICAL: Insert asm volatile with register clobbering
         * This forces the compiler to save/restore specific registers
         */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* External function call - cannot be inlined */
        clobber_func1();
        
        /* Another asm with different clobbers */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm6", "xmm7", "xmm8", "xmm9", "xmm10");
        
        /* Second external call */
        clobber_func2();
        
        /* More register clobbering */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13",
                     "xmm11", "xmm12", "xmm13", "xmm14", "xmm15");
        
        /* Third external call */
        clobber_func3();
        
        /* Post-call computations to keep variables live */
        int2 = int3 + int4 * (i + 1);
        int5 = int6 ^ int7;
        int8 = int9 | int10;
        
        f2 = f3 * f4 + (float)(i * 2);
        f5 = f1 / f3;
        
        d2 = d3 * d4 + (double)(i * 2);
        d5 = d1 / d3;
        
        vec2 = vec2 + vec3;
        vecd2 = vecd2 * (v2df){1.1, 1.1};
        
        *ptr1 = *ptr1 * 2;
        *ptr2 = *ptr2 / 2;
        
        ll2 = ll3 + ll4;
        ll3 = ll1 * 2;
        
        veci1 = veci1 * 2;
        
        /* Accumulate results to prevent elimination */
        sum += int1 + int2 + int3 + int4 + int5 + int6 + int7 + int8 + int9 + int10;
        fsum += f1 + f2 + f3 + f4 + f5;
        dsum += d1 + d2 + d3 + d4 + d5;
        
        /* More vector accumulation */
        for (int j = 0; j < 4; j++) {
            sum += veci1[j];
            fsum += vec1[j];
        }
        for (int j = 0; j < 2; j++) {
            dsum += vecd1[j];
        }
    }
    
    /* Final computation and output to prevent dead code elimination */
    double final_result = (double)sum + (double)fsum + dsum;
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
