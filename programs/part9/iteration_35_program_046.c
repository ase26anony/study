#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force caller-save behavior */
extern void clobber_func1(void);
extern void clobber_func2(void);
extern void clobber_func3(void);
extern void clobber_func4(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force specific register clobbering */
#define CLOBBER_INT_REGS asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15")
#define CLOBBER_FLOAT_REGS asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15")

int main(int argc, char **argv) {
    /* Force conditional control flow based on arguments */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE variables to prevent optimization */
    volatile int start_val = argc;
    
    /* Declare MANY local variables with mixed types to maximize register pressure */
    
    /* Integer variables (15+) */
    int i1 = start_val + 1;
    int i2 = start_val * 2;
    int i3 = start_val | 0xFF;
    int i4 = start_val ^ 0xAAAA;
    int i5 = start_val << 3;
    int i6 = ~start_val;
    int i7 = start_val + 100;
    int i8 = start_val - 50;
    int i9 = start_val * 3;
    int i10 = start_val / 2;
    int i11 = start_val % 7;
    int i12 = start_val & 0x5555;
    int i13 = start_val + 200;
    int i14 = start_val * start_val;
    int i15 = start_val | 0x1234;
    
    /* Floating point variables */
    float f1 = start_val * 1.1f;
    float f2 = start_val * 2.2f;
    float f3 = start_val * 3.3f;
    float f4 = start_val * 4.4f;
    float f5 = start_val * 5.5f;
    
    /* Double variables */
    double d1 = start_val * 1.111;
    double d2 = start_val * 2.222;
    double d3 = start_val * 3.333;
    double d4 = start_val * 4.444;
    
    /* Pointer variables */
    int *p1 = &i1;
    int *p2 = &i2;
    int *p3 = &i3;
    float *p4 = &f1;
    double *p5 = &d1;
    
    /* Vector variables - these use SSE/AVX registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f5};
    v4si vec3 = {i1, i2, i3, i4};
    v4si vec4 = {i5, i6, i7, i8};
    v2df vec5 = {d1, d2};
    v2df vec6 = {d3, d4};
    
    /* Additional volatile variables to ensure they stay live */
    volatile int vi1 = i1;
    volatile float vf1 = f1;
    volatile double vd1 = d1;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* Loop with conditional control flow to create complex basic blocks */
    for (int iter = 0; iter < iterations; iter++) {
        /* Complex pre-call computations keeping all variables live */
        
        /* Integer computations */
        i1 = i2 + i3 * iter;
        i2 = i4 ^ i5 | iter;
        i3 = i6 * i7 - iter;
        i4 = i8 & i9 | iter;
        i5 = i10 + i11 * iter;
        i6 = i12 - i13 / (iter + 1);
        i7 = i14 | i15 ^ iter;
        i8 = i1 * i2 + iter;
        i9 = i3 / (i4 + 1) * iter;
        i10 = i5 & i6 | iter;
        i11 = i7 ^ i8 - iter;
        i12 = i9 * i10 + iter;
        i13 = i11 | i12 & iter;
        i14 = i13 * i14 - iter;
        i15 = i15 + i1 * iter;
        
        /* Floating point computations */
        f1 = f2 * 1.5f + iter;
        f2 = f3 / 2.0f - iter;
        f3 = f4 * f5 + iter;
        f4 = f1 * 2.0f / (iter + 1);
        f5 = f2 + f3 - f4 * iter;
        
        /* Double computations */
        d1 = d2 * 1.75 + iter;
        d2 = d3 / 3.0 - iter;
        d3 = d4 * 2.5 + iter;
        d4 = d1 + d2 - d3 * iter;
        
        /* Pointer computations */
        *p1 = *p2 + *p3;
        *p2 = *p1 - iter;
        *p3 = *p2 | iter;
        *p4 = *p4 * 1.1f + iter;
        *p5 = *p5 / 2.0 + iter;
        
        /* Vector computations - use lots of vector registers */
        vec1 = vec1 + vec2 * (float)iter;
        vec2 = vec2 - vec1 / (float)(iter + 1);
        vec3 = vec3 | vec4 & iter;
        vec4 = vec4 + vec3 * iter;
        vec5 = vec5 + vec6 * (double)iter;
        vec6 = vec6 - vec5 / (double)(iter + 1);
        
        /* Force register spilling before call with aggressive clobbering */
        CLOBBER_INT_REGS;
        CLOBBER_FLOAT_REGS;
        
        /* External function call - forces caller-save */
        clobber_func1();
        
        /* More clobbering between calls */
        CLOBBER_INT_REGS;
        
        /* Second external call */
        clobber_func2();
        
        /* Conditional branch to create more complex CFG */
        if (iter % 2 == 0) {
            CLOBBER_FLOAT_REGS;
            clobber_func3();
            
            /* More computations in the branch */
            vec1 = vec1 * 2.0f;
            vec2 = vec2 / 3.0f;
            f1 = f1 + f2 * 2.0f;
        } else {
            CLOBBER_INT_REGS;
            clobber_func4();
            
            /* Different computations in the else branch */
            vec3 = vec3 | 0xFF;
            vec4 = vec4 & 0xFFFF;
            i1 = i1 * 2 + 1;
        }
        
        /* Post-call computations keeping variables live */
        i1 = i1 + *p1;
        i2 = i2 - *p2;
        f1 = f1 + *p4;
        d1 = d1 + *p5;
        
        /* More vector operations */
        vec1 = vec1 + (v4sf){f1, f2, f3, f4};
        vec5 = vec5 + (v2df){d1, d2};
        
        /* Accumulate results to prevent dead code elimination */
        total_result += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
                       i11 + i12 + i13 + i14 + i15 +
                       f1 + f2 + f3 + f4 + f5 +
                       d1 + d2 + d3 + d4 +
                       vec1[0] + vec1[1] + vec1[2] + vec1[3] +
                       vec5[0] + vec5[1];
        
        /* Force another clobber at end of iteration */
        CLOBBER_INT_REGS;
        CLOBBER_FLOAT_REGS;
    }
    
    /* Final computation and output to prevent optimization */
    total_result += vi1 + vf1 + vd1;
    
    printf("Result: %f\n", total_result);
    
    return (int)total_result % 256;
}
