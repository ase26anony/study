#include <stdio.h>
#include <stdlib.h>

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
    
    /* VOLATILE VARIABLES - prevent optimization */
    volatile int start_val = argc;
    
    /* MANY LOCAL VARIABLES of mixed types to create register pressure */
    
    /* Integer variables */
    int i1 = start_val + 1;
    int i2 = start_val * 2;
    int i3 = start_val | 0xFF;
    int i4 = start_val ^ 0xAA;
    int i5 = start_val << 2;
    int i6 = start_val + 100;
    int i7 = start_val - 50;
    int i8 = start_val * 3;
    int i9 = start_val / 2;
    int i10 = start_val % 7;
    
    /* Floating point variables */
    float f1 = start_val * 1.1f;
    float f2 = start_val * 2.2f;
    float f3 = start_val * 3.3f;
    float f4 = start_val * 4.4f;
    double d1 = start_val * 1.11;
    double d2 = start_val * 2.22;
    double d3 = start_val * 3.33;
    
    /* Pointer variables */
    int *p1 = &i1;
    int *p2 = &i2;
    float *p3 = &f1;
    double *p4 = &d1;
    char *p5 = (char*)argv[0];
    
    /* Vector variables - use all vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f1};
    v4sf vec3 = {f3, f4, f1, f2};
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d3};
    v4si ivec1 = {i1, i2, i3, i4};
    v4si ivec2 = {i5, i6, i7, i8};
    
    /* Additional variables to ensure spilling */
    long long ll1 = start_val * 1000LL;
    long long ll2 = start_val * 2000LL;
    short s1 = start_val;
    short s2 = start_val * 2;
    char c1 = start_val;
    char c2 = start_val + 1;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* LOOP with complex control flow */
    for (int iter = 0; iter < iterations; iter++) {
        /* PRE-CALL COMPUTATIONS - keep variables live */
        
        /* Integer computations */
        i1 = i1 * 2 + iter;
        i2 = i2 ^ (i1 << 1);
        i3 = i3 + i2 - iter;
        i4 = i4 | (i3 & 0xFF);
        i5 = (i5 << 3) | (iter & 0xF);
        i6 = i6 + i5 - i4;
        i7 = i7 * 2 + iter;
        i8 = i8 ^ (iter * 0xABCD);
        i9 = i9 + (iter % 5);
        i10 = i10 * 3 - iter;
        
        /* Floating point computations */
        f1 = f1 * 1.5f + iter;
        f2 = f2 / 1.1f - iter;
        f3 = f3 + f1 * f2;
        f4 = f4 - f3 / 2.0f;
        d1 = d1 * 1.25 + iter;
        d2 = d2 / 1.05 - iter;
        d3 = d3 + d1 * d2;
        
        /* Vector computations - use all vector registers */
        vec1 = vec1 + vec2 * (float)iter;
        vec2 = vec2 - vec3 / (float)(iter + 1);
        vec3 = vec3 * vec1 + vec2;
        dvec1 = dvec1 + dvec2 * (double)iter;
        dvec2 = dvec2 - dvec1 / (double)(iter + 2);
        ivec1 = ivec1 + ivec2 * iter;
        ivec2 = ivec2 - ivec1 / (iter + 1);
        
        /* Pointer computations */
        *p1 = *p1 + iter;
        *p2 = *p2 - iter;
        *p3 = *p3 * (1.0f + iter * 0.1f);
        *p4 = *p4 / (1.0 + iter * 0.05);
        
        /* Additional type mixing */
        ll1 = ll1 + (long long)i1 * iter;
        ll2 = ll2 - (long long)i2 * iter;
        s1 = s1 + (short)iter;
        s2 = s2 - (short)iter;
        c1 = c1 ^ (char)iter;
        c2 = c2 | (char)iter;
        
        /* ASM to clobber INTEGER registers - force save/restore */
        asm volatile ("" 
                     : /* no outputs */
                     : /* no inputs */
                     : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                       "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                       "memory");
        
        /* EXTERNAL FUNCTION CALL - forces caller-save */
        clobber_func1();
        
        /* ASM to clobber VECTOR registers */
        asm volatile (""
                     : /* no outputs */
                     : /* no inputs */
                     : "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                       "xmm6", "xmm7", "xmm8", "xmm9", "xmm10", "xmm11",
                       "xmm12", "xmm13", "xmm14", "xmm15",
                       "memory");
        
        /* More computations to keep variables live across calls */
        i1 = i1 ^ i2;
        i2 = i2 + i3;
        i3 = i3 * i4;
        i4 = i4 | i5;
        
        f1 = f1 + f2 * 0.5f;
        f2 = f2 - f3 / 1.5f;
        f3 = f3 * f4;
        f4 = f4 / 2.0f;
        
        vec1 = vec1 + vec3;
        vec2 = vec2 * vec1;
        vec3 = vec3 - vec2;
        
        /* Second ASM with different clobber list */
        asm volatile (""
                     : /* no outputs */
                     : /* no inputs */
                     : "rax", "rbx", "xmm0", "xmm1", "xmm2", "xmm3",
                       "memory");
        
        /* Another external call */
        clobber_func2();
        
        /* Mixed clobber */
        asm volatile (""
                     : /* no outputs */
                     : /* no inputs */
                     : "rcx", "rdx", "xmm4", "xmm5", "xmm6", "xmm7",
                       "memory");
        
        /* Conditional call based on iteration */
        if (iter % 2 == 0) {
            clobber_func3();
            
            /* More register clobbering */
            asm volatile (""
                         : /* no outputs */
                         : /* no inputs */
                         : "r8", "r9", "r10", "xmm8", "xmm9", "xmm10",
                           "memory");
        }
        
        /* POST-CALL COMPUTATIONS - variables must be restored */
        
        /* Complex computations using all variables */
        i5 = i5 + (i1 * i2) / (i3 + 1);
        i6 = i6 ^ (i4 | i5);
        i7 = i7 * 2 + (i6 % 17);
        i8 = i8 + i7 - i5;
        i9 = i9 * 3 - (i8 & 0xFFF);
        i10 = i10 + (i9 << 2);
        
        f1 = f1 * 2.0f + d1;
        f2 = f2 / 1.1f - d2;
        f3 = f3 + f4 * d3;
        f4 = f4 - f1 / f2;
        
        d1 = d1 + (double)f1 * 0.25;
        d2 = d2 - (double)f2 * 0.33;
        d3 = d3 * 1.01 + (double)f3;
        
        vec1 = vec1 * 1.1f + vec2;
        vec2 = vec2 - vec3 * 0.9f;
        vec3 = vec3 + vec1 / 2.0f;
        
        dvec1 = dvec1 + dvec2 * 0.5;
        dvec2 = dvec2 - dvec1 / 3.0;
        
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec2 - ivec1;
        
        /* Pointer updates */
        *p1 = *p1 + 1;
        *p2 = *p2 - 1;
        *p3 = *p3 * 1.05f;
        *p4 = *p4 / 1.02;
        
        /* Accumulate results to prevent dead code elimination */
        total_result += i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10;
        total_result += f1 + f2 + f3 + f4;
        total_result += d1 + d2 + d3;
        total_result += vec1[0] + vec1[1] + vec1[2] + vec1[3];
        total_result += vec2[0] + vec2[1] + vec2[2] + vec2[3];
        total_result += dvec1[0] + dvec1[1];
        total_result += ivec1[0] + ivec1[1] + ivec1[2] + ivec1[3];
        total_result += ll1 + ll2;
        total_result += s1 + s2 + c1 + c2;
    }
    
    /* Final output to prevent optimization */
    printf("Result: %f\n", total_result);
    
    return (int)total_result % 256;
}
