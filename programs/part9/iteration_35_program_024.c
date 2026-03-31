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

/* Force memory barriers and register clobbering */
#define CLOBBER_REGS_1 asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", "xmm0", "xmm1", "xmm2")
#define CLOBBER_REGS_2 asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9", "xmm3", "xmm4", "xmm5")
#define CLOBBER_REGS_3 asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13", "xmm6", "xmm7", "xmm8")
#define CLOBBER_REGS_4 asm volatile ("" ::: "memory", "r14", "r15", "xmm9", "xmm10", "xmm11", "xmm12")

int main(int argc, char *argv[]) {
    /* Force argc check for conditional control flow */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables of mixed types to maximize register pressure */
    
    /* Integer variables */
    volatile int v1 = 1;
    volatile int v2 = 2;
    volatile int v3 = 3;
    volatile int v4 = 4;
    volatile int v5 = 5;
    volatile int v6 = 6;
    volatile int v7 = 7;
    volatile int v8 = 8;
    volatile int v9 = 9;
    volatile int v10 = 10;
    
    /* Floating point variables */
    volatile float f1 = 1.1f;
    volatile float f2 = 2.2f;
    volatile float f3 = 3.3f;
    volatile float f4 = 4.4f;
    volatile double d1 = 1.11;
    volatile double d2 = 2.22;
    volatile double d3 = 3.33;
    volatile double d4 = 4.44;
    
    /* Pointer variables */
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *p3 = &f1;
    volatile double *p4 = &d1;
    
    /* Vector variables */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    /* Additional variables for more pressure */
    volatile long l1 = 100;
    volatile long l2 = 200;
    volatile short s1 = 300;
    volatile short s2 = 400;
    volatile char c1 = 'a';
    volatile char c2 = 'b';
    
    /* Result accumulator */
    volatile double total = 0.0;
    
    /* Loop to create complex control flow */
    for (volatile int i = 0; i < iterations; i++) {
        /* Phase 1: Intensive computations before call */
        v1 = v2 * v3 + i;
        v4 = v5 ^ v6 | v7;
        f1 = f2 * f3 + (float)i;
        d1 = d2 / d3 - (double)v1;
        
        /* Vector operations */
        vec1 = vec1 + vec2;
        vec3 = vec3 * vec1;
        dvec1 = dvec1 - dvec2;
        ivec1 = ivec1 & ivec2;
        
        /* Pointer arithmetic */
        p1 = p1 + (v1 & 0x3);
        p3 = p3 + 1;
        
        /* Mix types */
        l1 = (long)v1 * (long)v2;
        s1 = (short)(f1 * 10.0f);
        
        /* Force register clobbering before external call */
        CLOBBER_REGS_1;
        
        /* External call - forces caller-save */
        clobber_func1();
        
        /* More clobbering */
        CLOBBER_REGS_2;
        
        /* Phase 2: More computations between calls */
        v8 = v9 * v10 - v1;
        v2 = v3 << 2;
        f4 = f1 + f2 * f3;
        d4 = d1 * d2 + d3;
        
        vec2 = vec2 * vec3;
        dvec2 = dvec1 + dvec2;
        ivec2 = ivec1 | ivec2;
        
        /* Conditional to create branch around save/restore */
        if (i % 2 == 0) {
            CLOBBER_REGS_3;
            clobber_func2();
            CLOBBER_REGS_4;
            
            /* More computations in this branch */
            v5 = v6 + v7 * v8;
            f2 = f3 - f4;
            vec1 = vec1 * 2.0f;
        } else {
            CLOBBER_REGS_2;
            clobber_func3();
            CLOBBER_REGS_1;
            
            /* Different computations in else branch */
            v6 = v7 ^ v8;
            f3 = f4 / 2.0f;
            dvec1 = dvec2 * 0.5;
        }
        
        /* Phase 3: Final computations after calls */
        v9 = v10 + v1 * v2;
        v3 = v4 >> 1;
        d3 = d4 * 0.333;
        
        vec3 = vec1 + vec2;
        ivec1 = ivec2 << 1;
        
        /* Accumulate results to keep everything live */
        total += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        total += f1 + f2 + f3 + f4;
        total += d1 + d2 + d3 + d4;
        total += vec1[0] + vec2[1] + vec3[2];
        total += dvec1[0] + dvec2[1];
        total += ivec1[0] + ivec2[1];
        total += l1 + l2 + s1 + s2 + c1 + c2;
        
        /* Another external call at loop end */
        CLOBBER_REGS_4;
        clobber_func4();
        CLOBBER_REGS_3;
    }
    
    /* Final computation and output to prevent elimination */
    volatile double final_result = total / (iterations * 50.0);
    printf("Result: %f\n", final_result);
    
    return (int)final_result;
}
