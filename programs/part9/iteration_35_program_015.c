#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* External functions to force caller-save behavior */
extern void foo(void);
extern void bar(void);
extern void baz(void);

/* Vector types to increase register pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Prevent inlining and force register saves */
__attribute__((noinline)) 
static void clobber_registers(int variant) {
    if (variant == 0) {
        /* Clobber integer and vector registers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                                       "xmm0", "xmm1", "xmm2", "xmm3");
    } else if (variant == 1) {
        /* Clobber different registers */
        asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                                       "xmm4", "xmm5", "xmm6", "xmm7");
    } else {
        /* Clobber all caller-saved registers */
        asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx",
                                       "rsi", "rdi", "r8", "r9", "r10", "r11",
                                       "xmm0", "xmm1", "xmm2", "xmm3",
                                       "xmm4", "xmm5", "xmm6", "xmm7",
                                       "xmm8", "xmm9", "xmm10", "xmm11",
                                       "xmm12", "xmm13", "xmm14", "xmm15");
    }
}

int main(int argc, char *argv[]) {
    /* Force conditional control flow */
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* VOLATILE variables to prevent optimization */
    volatile int start_val = argc;
    
    /* Declare MANY local variables of mixed types to create register pressure */
    
    /* Integer variables */
    volatile int v1 = start_val + 1;
    volatile int v2 = start_val * 2;
    volatile int v3 = start_val | 0xFF;
    volatile int v4 = start_val ^ 0xAA;
    volatile int v5 = start_val << 2;
    volatile int v6 = ~start_val;
    volatile int v7 = start_val + 100;
    volatile int v8 = start_val - 50;
    
    /* Floating point variables */
    volatile float f1 = start_val * 1.1f;
    volatile float f2 = start_val * 2.2f;
    volatile float f3 = start_val * 3.3f;
    volatile float f4 = start_val * 4.4f;
    volatile double d1 = start_val * 1.11;
    volatile double d2 = start_val * 2.22;
    volatile double d3 = start_val * 3.33;
    
    /* Pointer variables */
    int arr1[10];
    int arr2[10];
    volatile int *p1 = arr1;
    volatile int *p2 = arr2;
    volatile int *p3 = &v1;
    volatile int *p4 = &v2;
    
    /* Vector variables - use all vector registers */
    v4sf vec1 = {f1, f2, f3, f4};
    v4sf vec2 = {f2, f3, f4, f1};
    v4sf vec3 = {f3, f4, f1, f2};
    v4sf vec4 = {f4, f1, f2, f3};
    
    v2df dvec1 = {d1, d2};
    v2df dvec2 = {d2, d3};
    v2df dvec3 = {d3, d1};
    
    v4si ivec1 = {v1, v2, v3, v4};
    v4si ivec2 = {v5, v6, v7, v8};
    v4si ivec3 = {v2, v3, v4, v5};
    
    /* Additional variables to ensure spill */
    volatile long l1 = start_val * 1000L;
    volatile long l2 = start_val * 2000L;
    volatile short s1 = start_val;
    volatile short s2 = start_val * 2;
    volatile char c1 = start_val;
    volatile char c2 = start_val + 1;
    
    /* Result accumulator */
    volatile double result = 0.0;
    
    /* Loop to create complex control flow */
    for (int i = 0; i < iterations; i++) {
        /* Complex computations before call to keep variables live */
        v1 = v1 * 2 + i;
        v2 = v2 ^ v1;
        v3 = v3 | (v2 << 3);
        v4 = v4 + v3 - i;
        
        f1 = f1 * 1.5f + i;
        f2 = f2 / 1.7f - i;
        d1 = d1 * 1.23 + i;
        d2 = d2 / 1.45 - i;
        
        /* Vector operations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec4;
        vec4 = vec4 / (v4sf){2.0f, 2.0f, 2.0f, 2.0f};
        
        dvec1 = dvec1 + dvec2;
        dvec2 = dvec2 * dvec3;
        
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec2 | ivec3;
        
        /* Pointer arithmetic */
        p1 = arr1 + i;
        p2 = arr2 + (iterations - i);
        *p3 = v1;
        *p4 = v2;
        
        /* Force register clobbering before call */
        clobber_registers(i % 3);
        
        /* External function call - forces caller-save */
        if (i % 2 == 0) {
            foo();
        } else if (i % 3 == 0) {
            bar();
        } else {
            baz();
        }
        
        /* More register clobbering after call */
        asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", 
                                   "xmm2", "xmm3", "xmm4", "xmm5");
        
        /* Complex computations after call using all variables */
        v5 = v5 + v1 * v2;
        v6 = v6 ^ (v3 | v4);
        v7 = v7 * 3 - v5;
        v8 = v8 / 2 + v6;
        
        f3 = f3 + f1 * f2;
        f4 = f4 - f3 / f1;
        d3 = d3 * d1 + d2;
        
        /* More vector operations */
        vec1 = vec1 * (v4sf){f1, f2, f3, f4};
        vec2 = vec2 + vec1;
        
        dvec3 = dvec1 * dvec2 + dvec3;
        
        ivec3 = ivec1 & ivec2;
        
        /* Update result with all variables to keep them live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        result += f1 + f2 + f3 + f4;
        result += d1 + d2 + d3;
        result += l1 + l2 + s1 + s2 + c1 + c2;
        
        /* Additional clobbering in the middle of computations */
        if (i % 2 == 1) {
            asm volatile ("" ::: "memory", "r12", "r13", "r14", "r15",
                                       "xmm6", "xmm7", "xmm8", "xmm9");
        }
        
        /* Another external call with different clobbering */
        clobber_registers((i + 1) % 3);
        foo();
        
        /* Final computations */
        vec3 = vec3 + vec4 * 0.5f;
        vec4 = vec4 - vec1 * 0.25f;
        
        result += vec1[0] + vec2[1] + vec3[2] + vec4[3];
        result += dvec1[0] + dvec2[1] + dvec3[0];
        result += ivec1[0] + ivec2[1] + ivec3[2];
    }
    
    /* Use all variables one more time to ensure they're live */
    volatile double final_result = result;
    final_result += *p1 + *p2 + *p3 + *p4;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
