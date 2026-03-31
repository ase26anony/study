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
    
    /* Additional variables for more pressure */
    volatile long long ll1 = 100;
    volatile long long ll2 = 200;
    volatile long long ll3 = 300;
    volatile long long ll4 = 400;
    
    volatile int result = 0;
    
    /* Loop to create control flow and repeated save/restore opportunities */
    for (int i = 0; i < iterations; i++) {
        /* Complex computations before call to keep variables live */
        int1 = int2 + int3 * i;
        int4 = int5 - int6 / (i + 1);
        f1 = f2 * f3 + (float)i;
        d1 = d2 / d3 - (double)i;
        
        /* Vector operations */
        vec1 = vec1 + vec2 * (float)(i + 1);
        vec3 = vec3 - vec1;
        dvec1 = dvec1 * dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Pointer arithmetic */
        *ptr1 = *ptr2 + i;
        *fptr1 = *fptr2 * (float)(i + 1);
        
        /* Long long operations */
        ll1 = ll2 + ll3 * i;
        ll4 = ll1 - ll2;
        
        /* ASM to clobber specific integer registers */
        asm volatile ("" : : : "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi");
        
        /* First external call - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber specific floating point/vector registers */
        asm volatile ("" : : : "memory", "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* More computations between calls */
        int7 = int8 * int9 + int10;
        f3 = f4 * f5 - f1;
        d3 = d4 + d1 * d2;
        
        /* Second ASM with different clobbers */
        asm volatile ("" : : : "memory", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* Second external call */
        clobber_func2();
        
        /* ASM clobbering mixed registers */
        asm volatile ("" : : : "memory", "rax", "rbx", "xmm8", "xmm9", "xmm10", "xmm11");
        
        /* More computations to keep variables live */
        vec2 = vec1 + vec3;
        dvec2 = dvec1 * 2.0;
        ivec2 = ivec1 - ivec2;
        
        /* Third external call */
        clobber_func3();
        
        /* Final ASM clobber */
        asm volatile ("" : : : "memory", "ymm0", "ymm1", "ymm2", "ymm3");
        
        /* Complex conditional to create basic block boundaries */
        if (i % 2 == 0) {
            /* Even iteration computations */
            int2 = int3 * int4 + int1;
            f2 = f3 / f4 * f5;
            *ptr2 = *ptr1 + i * 2;
            
            /* Additional vector ops */
            vec1 = vec2 * vec3;
            ivec1 = ivec2 << 1;
        } else {
            /* Odd iteration computations */
            int3 = int4 - int5 * int2;
            f4 = f5 + f1 - f2;
            *fptr2 = *fptr1 / (float)(i + 2);
            
            /* Different vector ops */
            dvec1 = dvec2 / 3.0;
            ivec2 = ivec1 >> 1;
        }
        
        /* Accumulate result to prevent optimization */
        result += int1 + int2 + int3 + int4 + (int)f1 + (int)d1 + 
                  (int)vec1[0] + (int)dvec1[0] + ivec1[0] + (int)(ll1 % 100);
    }
    
    /* Use all variables in final computation to ensure they stay live */
    int final_result = result + int5 + int6 + int7 + int8 + int9 + int10 +
                      (int)f2 + (int)f3 + (int)f4 + (int)f5 +
                      (int)d2 + (int)d3 + (int)d4 +
                      (int)vec2[1] + (int)vec3[2] + (int)dvec2[1] +
                      ivec2[2] + (int)ll2 + (int)ll3 + (int)ll4 +
                      *ptr1 + *ptr2 + (int)*fptr1 + (int)*fptr2;
    
    printf("Result: %d\n", final_result);
    
    return final_result % 256;
}
