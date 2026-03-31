#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
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
#define CLOBBER_INT_REGS asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi")
#define CLOBBER_FLOAT_REGS asm volatile("" ::: "memory", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5")
#define CLOBBER_ALL_REGS asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", "rsi", "rdi", \
                                      "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7")

/* Volatile variables to prevent optimization */
static volatile int global_counter = 0;

int main(int argc, char *argv[]) {
    /* Phase 1: Declare many variables of mixed types to create register pressure */
    
    /* Integer variables */
    volatile int v1 = argc + 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc + 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc + 5;
    volatile long v6 = (long)argc * 100;
    volatile long v7 = (long)argc * 200;
    volatile uint64_t v8 = (uint64_t)argc * 300;
    volatile uint64_t v9 = (uint64_t)argc * 400;
    
    /* Floating point variables */
    volatile float f1 = (float)argc * 1.1f;
    volatile float f2 = (float)argc * 2.2f;
    volatile float f3 = (float)argc * 3.3f;
    volatile double d1 = (double)argc * 1.111;
    volatile double d2 = (double)argc * 2.222;
    volatile double d3 = (double)argc * 3.333;
    
    /* Pointer variables */
    volatile char *p1 = argv[0];
    volatile char *p2 = (argc > 1) ? argv[1] : argv[0];
    volatile int *p3 = &v1;
    volatile float *p4 = &f1;
    
    /* Vector variables */
    volatile v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    volatile v2df vec3 = {1.0, 2.0};
    volatile v2df vec4 = {3.0, 4.0};
    volatile v4si vec5 = {1, 2, 3, 4};
    volatile v4si vec6 = {5, 6, 7, 8};
    
    /* Additional variables for more pressure */
    volatile int v10 = 0, v11 = 0, v12 = 0, v13 = 0, v14 = 0;
    volatile float f4 = 0.0f, f5 = 0.0f, f6 = 0.0f;
    volatile double d4 = 0.0, d5 = 0.0;
    
    /* Phase 2: Complex conditional control flow with loops */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    volatile double result = 0.0;
    
    for (int i = 0; i < iterations; i++) {
        /* Pre-call computations using all variables to keep them live */
        if (i % 2 == 0) {
            /* Even iteration: integer and vector operations */
            v1 = v2 + v3 * i;
            v4 = v5 ^ v6;
            v7 = v8 >> (i % 4);
            v9 = v10 * v11 + i;
            
            vec5 = vec5 + vec6 * i;
            vec1 = vec1 + vec2 * (float)i;
            vec3 = vec3 + vec4 * (double)i;
            
            /* Pointer arithmetic */
            p1 = p1 + i;
            p3 = &v1 + i;
        } else {
            /* Odd iteration: floating point operations */
            f1 = f2 * f3 + (float)i;
            f4 = f5 - f6 * (float)i;
            d1 = d2 / d3 + (double)i;
            d4 = d5 * (double)i - 1.0;
            
            vec2 = vec1 * (float)(i + 1);
            vec4 = vec3 * (double)(i + 1);
        }
        
        /* Mix scalar and vector operations */
        v12 = v1 + (int)f1 + vec5[0];
        v13 = v4 + (int)d1 + vec6[1];
        f6 = (float)v7 + f3 + vec1[2];
        d5 = (double)v8 + d3 + vec3[1];
        
        /* Force register clobbering before call */
        CLOBBER_INT_REGS;
        
        /* Conditional function calls to create complex basic blocks */
        if (i % 3 == 0) {
            clobber_func1();
            CLOBBER_FLOAT_REGS;
            
            /* More computations between calls */
            v14 = v12 * v13 - i;
            vec6 = vec5 + v14;
            
            clobber_func2();
            CLOBBER_ALL_REGS;
        } else if (i % 3 == 1) {
            clobber_func3();
            
            /* Different computation pattern */
            f5 = f4 * 2.0f + (float)v10;
            vec2 = vec1 * f5;
            
            CLOBBER_INT_REGS;
            clobber_func4();
        } else {
            /* Nested condition for more complexity */
            if (v1 > 100) {
                CLOBBER_ALL_REGS;
                clobber_func1();
                clobber_func3();
            } else {
                CLOBBER_FLOAT_REGS;
                clobber_func2();
                clobber_func4();
            }
        }
        
        /* Post-call computations to keep variables live */
        v10 = v11 + v12 * v13;
        v11 = v10 ^ v14;
        
        f4 = f5 + f6 * 3.14f;
        f5 = f4 - f1;
        
        d4 = d5 * 2.71828;
        d5 = d4 / d1;
        
        vec1 = vec2 + vec1;
        vec3 = vec4 * 0.5 + vec3;
        vec5 = vec6 | vec5;
        
        /* Accumulate result to prevent dead code elimination */
        result += (double)v1 + (double)f1 + d1 + (double)vec5[0] + vec3[0];
        
        /* Additional clobbering to force more save/restore */
        if (i % 2 == 0) {
            asm volatile("" ::: "memory", "r8", "r9", "r10", "r11", 
                        "xmm8", "xmm9", "xmm10", "xmm11");
        }
    }
    
    /* Phase 3: Final aggregation and output */
    volatile double final_result = result;
    
    /* Use all variables one more time */
    final_result += (double)v2 + (double)v3 + (double)v4 + (double)v5;
    final_result += (double)v6 + (double)v7 + (double)v8 + (double)v9;
    final_result += (double)f2 + (double)f3 + d2 + d3;
    final_result += (double)vec2[0] + (double)vec4[0] + (double)vec6[0];
    
    /* Pointer-based computations */
    if (p1 != NULL) final_result += (double)(*p1);
    if (p3 != NULL) final_result += (double)(*p3);
    
    printf("Result: %f\n", final_result);
    
    /* Force one more clobber before return */
    CLOBBER_ALL_REGS;
    
    return (int)final_result % 256;
}

/* Dummy external function definitions to satisfy linker */
void clobber_func1(void) {
    asm volatile("" ::: "memory");
}

void clobber_func2(void) {
    asm volatile("" ::: "memory");
}

void clobber_func3(void) {
    asm volatile("" ::: "memory");
}

void clobber_func4(void) {
    asm volatile("" ::: "memory");
}
