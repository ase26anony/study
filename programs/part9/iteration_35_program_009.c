/* test_caller_save.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* External functions to force calls */
extern void external_func1(void);
extern void external_func2(void);
extern void external_func3(void);

/* Vector types for SSE/AVX pressure */
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

/* Force non-inline, register-clobbering behavior */
__attribute__((noinline)) 
void clobber_registers_a(void) {
    asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", 
                  "xmm0", "xmm1", "xmm2", "xmm3", "memory");
}

__attribute__((noinline))
void clobber_registers_b(void) {
    asm volatile ("" : : : "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
                  "xmm4", "xmm5", "xmm6", "xmm7", "xmm8", "xmm9", "memory");
}

__attribute__((noinline))
void clobber_vector_regs(void) {
    /* Clobber all xmm/ymm registers */
    asm volatile ("" : : : 
        "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
        "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13", "xmm14", "xmm15",
        "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7",
        "ymm8", "ymm9", "ymm10", "ymm11", "ymm12", "ymm13", "ymm14", "ymm15",
        "memory");
}

int main(int argc, char *argv[]) {
    /* Force conditional control flow */
    volatile int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10;
    
    /* Declare MANY local variables with mixed types to maximize register pressure */
    
    /* Integer variables (general purpose registers) */
    volatile int i1 = 1, i2 = 2, i3 = 3, i4 = 4, i5 = 5;
    volatile long l1 = 100, l2 = 200, l3 = 300, l4 = 400, l5 = 500;
    volatile unsigned int u1 = 0xAAAAAAAA, u2 = 0xBBBBBBBB, u3 = 0xCCCCCCCC;
    
    /* Floating point variables (xmm registers) */
    volatile float f1 = 1.1f, f2 = 2.2f, f3 = 3.3f, f4 = 4.4f, f5 = 5.5f;
    volatile double d1 = 1.111, d2 = 2.222, d3 = 3.333, d4 = 4.444, d5 = 5.555;
    
    /* Pointer variables */
    volatile char *p1 = (char*)&i1;
    volatile int *p2 = &i2;
    volatile float *p3 = &f1;
    volatile double *p4 = &d1;
    
    /* Vector variables (SSE/AVX registers) */
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    v2df dvec1 = {1.23, 4.56};
    v2df dvec2 = {7.89, 10.11};
    v4si ivec1 = {100, 200, 300, 400};
    v4si ivec2 = {500, 600, 700, 800};
    
    /* More variables to ensure spilling */
    volatile int i6 = 6, i7 = 7, i8 = 8, i9 = 9, i10 = 10;
    volatile float f6 = 6.6f, f7 = 7.7f, f8 = 8.8f, f9 = 9.9f, f10 = 10.10f;
    volatile double d6 = 6.666, d7 = 7.777, d8 = 8.888, d9 = 9.999, d10 = 10.1010;
    
    /* Accumulator for final result */
    double total = 0.0;
    
    /* Complex loop with conditional control flow */
    for (volatile int loop = 0; loop < iterations; loop++) {
        /* Pre-call computations - keep all variables live */
        i1 = i2 + i3;
        i2 = i3 * i4;
        i3 = i4 ^ i5;
        i4 = i5 - i1;
        i5 = i1 | i2;
        
        l1 = l2 + l3;
        l2 = l3 - l4;
        l3 = l4 * l5;
        l4 = l5 / (l1 + 1);
        l5 = l1 ^ l2;
        
        u1 = u2 & u3;
        u2 = u3 | u1;
        u3 = u1 ^ u2;
        
        f1 = f2 + f3;
        f2 = f3 * f4;
        f3 = f4 - f5;
        f4 = f5 / (f1 + 1.0f);
        f5 = f1 * f2;
        
        d1 = d2 + d3;
        d2 = d3 * d4;
        d3 = d4 - d5;
        d4 = d5 / (d1 + 1.0);
        d5 = d1 * d2;
        
        /* Vector operations */
        vec1 = vec1 + vec2;
        vec2 = vec2 * vec3;
        vec3 = vec3 - vec1;
        dvec1 = dvec1 + dvec2;
        dvec2 = dvec2 * dvec1;
        ivec1 = ivec1 + ivec2;
        ivec2 = ivec2 - ivec1;
        
        /* Pointer arithmetic */
        p1 = (char*)((long)p1 + 1);
        p2 = (int*)((long)p2 + 4);
        p3 = (float*)((long)p3 + 4);
        p4 = (double*)((long)p4 + 8);
        
        /* More computations */
        i6 = i7 + i8;
        i7 = i8 * i9;
        i8 = i9 ^ i10;
        i9 = i10 - i6;
        i10 = i6 | i7;
        
        f6 = f7 + f8;
        f7 = f8 * f9;
        f8 = f9 - f10;
        f9 = f10 / (f6 + 1.0f);
        f10 = f6 * f7;
        
        d6 = d7 + d8;
        d7 = d8 * d9;
        d8 = d9 - d10;
        d9 = d10 / (d6 + 1.0);
        d10 = d6 * d7;
        
        /* Conditional branch to create complex CFG */
        if (loop % 2 == 0) {
            /* First call site with register clobbering */
            clobber_registers_a();
            external_func1();
            clobber_vector_regs();
            
            /* Interleaved computations */
            i1 = i1 + i6;
            f1 = f1 + f6;
            d1 = d1 + d6;
            vec1 = vec1 + (v4sf){f1, f2, f3, f4};
        } else {
            /* Second call site with different clobbering */
            clobber_registers_b();
            external_func2();
            clobber_vector_regs();
            
            /* Different computations */
            i2 = i2 + i7;
            f2 = f2 + f7;
            d2 = d2 + d7;
            vec2 = vec2 + (v4sf){f5, f6, f7, f8};
        }
        
        /* Third call site (always executed) */
        asm volatile ("" : : : "rax", "rbx", "rcx", "rdx", 
                      "xmm0", "xmm1", "xmm2", "xmm3", "memory");
        external_func3();
        asm volatile ("" : : : "rsi", "rdi", "r8", "r9",
                      "xmm4", "xmm5", "xmm6", "xmm7", "memory");
        
        /* Post-call computations - keep variables live */
        i3 = i3 + i8;
        i4 = i4 + i9;
        i5 = i5 + i10;
        
        f3 = f3 + f8;
        f4 = f4 + f9;
        f5 = f5 + f10;
        
        d3 = d3 + d8;
        d4 = d4 + d9;
        d5 = d5 + d10;
        
        l3 = l3 + (long)i3;
        l4 = l4 + (long)i4;
        l5 = l5 + (long)i5;
        
        vec3 = vec3 + vec1;
        dvec1 = dvec1 + dvec2;
        ivec1 = ivec1 + ivec2;
        
        /* Accumulate to prevent elimination */
        total += i1 + i2 + i3 + i4 + i5 + 
                f1 + f2 + f3 + f4 + f5 +
                d1 + d2 + d3 + d4 + d5 +
                (l1 + l2 + l3 + l4 + l5) / 1000.0;
    }
    
    /* Use all variables one more time */
    volatile double final_result = 
        i1 + i2 + i3 + i4 + i5 + i6 + i7 + i8 + i9 + i10 +
        f1 + f2 + f3 + f4 + f5 + f6 + f7 + f8 + f9 + f10 +
        d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 +
        l1 + l2 + l3 + l4 + l5 +
        u1 + u2 + u3 +
        vec1[0] + vec2[1] + vec3[2] +
        dvec1[0] + dvec2[1] +
        ivec1[0] + ivec2[1];
    
    total += final_result;
    
    printf("Result: %f\n", total);
    return (int)total % 256;
}
