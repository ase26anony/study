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

/* Prevent optimization of critical variables */
volatile int vol_int_guard = 0;

int main(int argc, char **argv) {
    /* Phase 1: Declare MANY variables of mixed types to maximize register pressure */
    
    /* Integer variables */
    volatile int v1 = argc + 1;
    int v2 = argc * 2;
    long v3 = (long)argc * 3;
    unsigned int v4 = (unsigned int)argc * 4;
    short v5 = (short)(argc * 5);
    char v6 = (char)(argc * 6);
    
    /* Floating point variables */
    float f1 = (float)argc * 1.1f;
    double d1 = (double)argc * 1.2;
    float f2 = (float)argc * 1.3f;
    double d2 = (double)argc * 1.4;
    long double ld1 = (long double)argc * 1.5L;
    
    /* Pointer variables */
    int *p1 = &v1;
    float *p2 = &f1;
    double *p3 = &d1;
    char *p4 = (char*)argv[0];
    
    /* Vector variables - these use SSE/AVX registers */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* More variables to ensure spill */
    int v7 = v1 + v2;
    int v8 = v3 + v4;
    float f3 = f1 + f2;
    double d3 = d1 + d2;
    v4sf vec3 = vec1 + vec2;
    v2df dvec3 = dvec1 + dvec2;
    
    /* Additional pressure variables */
    unsigned long v9 = (unsigned long)v1 * v2;
    float f4 = f1 * f2;
    double d4 = d1 * d2;
    int v10 = v5 + v6;
    v4si ivec3 = ivec1 + ivec2;
    
    /* Total: 30+ live variables across multiple register classes */
    
    /* Phase 2: Loop with conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; /* Bound iterations */
    
    /* Result accumulator */
    double total_result = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex conditional to create basic block boundaries */
        if (i % 2 == 0) {
            /* Branch 1: Heavy computation before call */
            vec1 = vec1 * vec2 + vec3;
            dvec1 = dvec1 * dvec2 + dvec3;
            ivec1 = ivec1 + ivec2 + ivec3;
            
            /* Mix scalar and vector operations */
            f1 = f1 * f2 + f3;
            d1 = d1 * d2 + d3;
            v1 = v1 * v2 + v7;
            v3 = v3 + v4 * v8;
            
            /* Force register clobbering before call */
            asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3");
            
            /* External call - forces caller-save */
            clobber_func1();
            
            /* More clobbering after call */
            asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
            
            /* Computation after call using same variables */
            vec2 = vec1 * 2.0f;
            dvec2 = dvec1 * 2.0;
            f2 = f1 * 2.0f;
            d2 = d1 * 2.0;
            v2 = v1 * 2;
            v4 = v3 * 2;
            
        } else {
            /* Branch 2: Different computation pattern */
            /* Use pointer arithmetic to keep pointers live */
            int temp1 = *p1 + v1;
            float temp2 = *p2 + f1;
            double temp3 = *p3 + d1;
            
            /* Vector operations */
            vec3 = vec1 + vec2;
            dvec3 = dvec1 + dvec2;
            ivec3 = ivec1 - ivec2;
            
            /* Clobber different registers */
            asm volatile("" ::: "memory", "r10", "r11", "r12", "r13", "r14", "r15",
                                       "xmm8", "xmm9", "xmm10", "xmm11");
            
            /* Another external call */
            clobber_func2();
            
            /* More clobbering */
            asm volatile("" ::: "memory", "xmm12", "xmm13", "xmm14", "xmm15");
            
            /* Post-call computation */
            *p1 = temp1 + v2;
            *p2 = temp2 + f2;
            *p3 = temp3 + d2;
            
            vec1 = vec3 * 0.5f;
            dvec1 = dvec3 * 0.5;
        }
        
        /* Common code with yet another call */
        if (i % 3 == 0) {
            /* Mix all variable types */
            v5 = v6 + v10;
            f3 = f4 * 0.3f;
            d3 = d4 * 0.3;
            vec2 = vec1 + vec3;
            
            /* Clobber critical registers */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
            
            clobber_func3();
            
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
        }
        
        /* Accumulate results to prevent elimination */
        total_result += (double)v1 + (double)v2 + (double)f1 + d1 + 
                       (double)vec1[0] + dvec1[0] + (double)ivec1[0];
        
        /* Force another call in loop tail */
        if (i < iterations - 1) {
            asm volatile("" ::: "memory", "rsi", "rdi", "xmm4", "xmm5");
            clobber_func4();
            asm volatile("" ::: "memory", "r8", "r9", "xmm6", "xmm7");
        }
    }
    
    /* Phase 3: Final computation using all variables */
    double final_result = 0.0;
    
    /* Use all integer variables */
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    
    /* Use all float variables */
    final_result += f1 + f2 + f3 + f4;
    
    /* Use all double variables */
    final_result += d1 + d2 + d3 + d4 + ld1;
    
    /* Use vector variables */
    final_result += vec1[0] + vec2[1] + vec3[2];
    final_result += dvec1[0] + dvec2[1] + dvec3[0];
    final_result += ivec1[0] + ivec2[1] + ivec3[2];
    
    /* Use pointer dereferences */
    if (p1) final_result += *p1;
    if (p2) final_result += *p2;
    if (p3) final_result += *p3;
    
    /* Mix with loop result */
    final_result += total_result;
    
    /* Prevent dead code elimination */
    if (vol_int_guard) {
        printf("Guard triggered: %f\n", final_result);
    }
    
    /* Print result to create side effect */
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
