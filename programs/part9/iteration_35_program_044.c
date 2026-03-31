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

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables of mixed types to maximize register pressure */
    
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
    
    /* Pointer variables */
    volatile int *ptr1 = &int1;
    volatile int *ptr2 = &int2;
    volatile float *fptr1 = &f1;
    volatile float *fptr2 = &f2;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional variables to ensure spill */
    volatile long long ll1 = 100;
    volatile long long ll2 = 200;
    volatile long long ll3 = 300;
    volatile long long ll4 = 400;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop to create complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations - keep all variables live */
        int1 = int2 * int3 + i;
        int4 = int5 ^ int6;
        int7 = int8 | int9;
        
        f1 = f2 * f3 + (float)i;
        f4 = f5 / 2.0f;
        
        d1 = d2 + d3 * 0.5;
        d4 = d1 - d2;
        
        /* Vector operations */
        vec1 = vec1 + vec2;
        vec3 = vec1 * vec2;
        dvec1 = dvec1 + dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Pointer arithmetic */
        *ptr1 = *ptr2 + int1;
        *fptr1 = *fptr2 * f1;
        
        /* Long long operations */
        ll1 = ll2 + ll3;
        ll4 = ll1 * 2;
        
        /* First asm volatile - clobber integer registers */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
        
        /* First external call - forces caller-save */
        clobber_func1();
        
        /* Second asm volatile - clobber floating point/vector registers */
        asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations between calls */
        int2 = int3 * int4 - i;
        int5 = int6 & int7;
        int8 = int9 ^ int10;
        
        f2 = f3 * f4 - (float)i;
        f5 = f1 / 3.0f;
        
        d2 = d3 - d4 * 0.25;
        d3 = d1 + d2;
        
        /* More vector operations */
        vec2 = vec3 - vec1;
        dvec2 = dvec1 * 2.0;
        ivec2 = ivec1 << 1;
        
        /* Third asm volatile - mixed clobber */
        asm volatile("" ::: "memory", "r8", "r9", "r10", "r11", 
                     "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* Second external call */
        clobber_func2();
        
        /* Post-call computations - ensure variables stay live */
        int3 = int4 + int5 * i;
        int6 = int7 | int8;
        int9 = int10 ^ int1;
        
        f3 = f4 + f5 * 2.0f;
        f1 = f2 / 4.0f;
        
        d4 = d1 * d2 + d3;
        d1 = d4 - d3;
        
        /* Final vector operations */
        vec3 = vec1 + vec2 * 2.0f;
        dvec1 = dvec2 / 2.0;
        ivec1 = ivec2 >> 1;
        
        /* Pointer updates */
        *ptr2 = *ptr1 - int2;
        *fptr2 = *fptr1 + f3;
        
        /* Long long updates */
        ll2 = ll3 * ll4;
        ll3 = ll1 + ll2;
        
        /* Fourth asm volatile - clobber everything */
        asm volatile("" ::: "memory", 
                     "rax", "rbx", "rcx", "rdx",
                     "xmm0", "xmm1", "xmm2", "xmm3",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Third external call */
        clobber_func3();
        
        /* Aggregate results to prevent elimination */
        total_result += (double)int1 + (double)f1 + d1 + 
                       (double)vec1[0] + (double)dvec1[0] + (double)ivec1[0] +
                       (double)*ptr1 + (double)*fptr1 + (double)ll1;
    }
    
    /* Final computation and output */
    total_result += (double)int10 + (double)f5 + d4 + 
                   (double)vec3[3] + (double)dvec2[1] + (double)ivec2[3];
    
    printf("Result: %f\n", total_result);
    
    return (int)total_result % 256;
}
