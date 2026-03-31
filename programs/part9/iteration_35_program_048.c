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

/* Prevent inlining and force register saves */
__attribute__((noinline)) 
static void use_variables(volatile int *trigger) {
    /* This function exists just to create more call sites */
    *trigger += 1;
}

int main(int argc, char **argv) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE VARIABLES - Prevent optimization */
    volatile int start = 0;
    
    /* MANY LOCAL VARIABLES - Create register pressure */
    /* Integer variables */
    int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    int i11 = 11, i12 = 12, i13 = 13, i14 = 14, i15 = 15;
    
    /* Floating point variables */
    float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    double d1 = 1.11, d2 = 2.22, d3 = 3.33, d4 = 4.44, d5 = 5.55;
    
    /* Pointer variables */
    int *p1 = &i1, *p2 = &i2, *p3 = &i3, *p4 = &i4, *p5 = &i5;
    
    /* Vector variables - use all vector registers */
    v4sf vf1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vf2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vf3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v4sf vf4 = {13.0f, 14.0f, 15.0f, 16.0f};
    
    v2df vd1 = {1.01, 2.02};
    v2df vd2 = {3.03, 4.04};
    v2df vd3 = {5.05, 6.06};
    v2df vd4 = {7.07, 8.08};
    
    v4si vi1 = {1, 2, 3, 4};
    v4si vi2 = {5, 6, 7, 8};
    v4si vi3 = {9, 10, 11, 12};
    v4si vi4 = {13, 14, 15, 16};
    
    /* Volatile accumulators to prevent dead code elimination */
    volatile int int_sum = 0;
    volatile float float_sum = 0.0f;
    volatile double double_sum = 0.0;
    volatile v4sf vector_sum = {0.0f, 0.0f, 0.0f, 0.0f};
    
    /* LOOP with conditional control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* COMPUTATION BEFORE CALL - Keep variables live */
        i1 = i2 + i3 * iter;
        i4 = i5 ^ i6;
        i7 = i8 | i9;
        i10 = i11 & i12;
        i13 = i14 << 2;
        i15 = i1 >> 1;
        
        f1 = f2 * f3 + (float)iter;
        f4 = f5 / f2;
        d1 = d2 + d3 * iter;
        d4 = d5 - d1;
        
        /* Vector operations */
        vf1 = vf2 + vf3 * (float)(iter + 1);
        vf4 = vf1 - vf2;
        vd1 = vd2 * vd3 + (double)iter;
        vd4 = vd1 / vd2;
        vi1 = vi2 & vi3;
        vi4 = vi1 | vi2;
        
        /* Pointer arithmetic */
        p1 = p2 + iter;
        p3 = p4 - iter;
        *p5 = iter;
        
        /* ASM to clobber integer registers */
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                      "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
        
        /* FUNCTION CALL - Forces caller-save */
        clobber_func1();
        
        /* ASM to clobber floating point/vector registers */
        asm volatile ("" : : : "xmm0", "xmm1", "xmm2", "xmm3", 
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "xmm8", "xmm9", "xmm10", "xmm11",
                      "xmm12", "xmm13", "xmm14", "xmm15",
                      "memory");
        
        /* COMPUTATION AFTER CALL - Variables must be restored */
        i2 = i3 + i4 * (iter + 2);
        i5 = i6 ^ i7;
        i8 = i9 | i10;
        i11 = i12 & i13;
        i14 = i15 << 1;
        
        f2 = f3 * f4 + (float)(iter * 2);
        f5 = f1 / f3;
        d2 = d3 + d4 * iter;
        d5 = d1 - d2;
        
        /* More vector operations */
        vf2 = vf3 + vf4 * (float)(iter + 3);
        vf3 = vf2 - vf1;
        vd2 = vd3 * vd4 + (double)(iter * 2);
        vd3 = vd2 / vd1;
        vi2 = vi3 & vi4;
        vi3 = vi1 | vi4;
        
        /* Conditional call based on iteration */
        if (iter % 2 == 0) {
            /* ASM with different clobber list */
            asm volatile ("" : : : "eax", "ebx", "ecx", "edx", "memory");
            clobber_func2();
        } else {
            asm volatile ("" : : : "ymm0", "ymm1", "ymm2", "ymm3", "memory");
            clobber_func3();
        }
        
        /* Accumulate results - forces variables to be used */
        int_sum += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
                  i11 + i12 + i13 + i14 + i15;
        
        float_sum += f1 + f2 + f3 + f4 + f5;
        double_sum += d1 + d2 + d3 + d4 + d5;
        
        /* Vector accumulation */
        vector_sum += vf1 + vf2 + vf3 + vf4;
        
        /* Another function call with mixed clobbering */
        use_variables(&start);
        
        /* Final asm to clobber everything */
        asm volatile ("" : : : 
                      "rax", "rbx", "rcx", "rdx",
                      "xmm0", "xmm1", "xmm2", "xmm3",
                      "xmm4", "xmm5", "xmm6", "xmm7",
                      "memory");
    }
    
    /* Final computation to use all variables */
    int final_int = int_sum + (int)float_sum + (int)double_sum;
    
    /* Use vector result */
    float vector_result = vector_sum[0] + vector_sum[1] + 
                         vector_sum[2] + vector_sum[3];
    
    /* Print results to prevent elimination */
    printf("Result: int=%d, float=%f, vector=%f\n", 
           final_int, (float_sum + (float)double_sum), vector_result);
    
    return final_int > 0 ? 0 : 1;
}
