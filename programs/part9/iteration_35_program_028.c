#include <stdio.h>
#include <stdlib.h>

// Declare external functions to force calls
extern void foo(void);
extern void bar(void);
extern void baz(void);

// Define vector types
typedef float v4sf __attribute__((vector_size(16)));
typedef double v2df __attribute__((vector_size(16)));
typedef int v4si __attribute__((vector_size(16)));

int main(int argc, char **argv) {
    // Force register pressure with many live variables
    volatile int v1 = argc * 1;
    volatile int v2 = argc * 2;
    volatile int v3 = argc * 3;
    volatile int v4 = argc * 4;
    volatile int v5 = argc * 5;
    volatile int v6 = argc * 6;
    volatile int v7 = argc * 7;
    volatile int v8 = argc * 8;
    
    volatile float f1 = argc * 1.1f;
    volatile float f2 = argc * 2.2f;
    volatile float f3 = argc * 3.3f;
    volatile float f4 = argc * 4.4f;
    
    volatile double d1 = argc * 1.11;
    volatile double d2 = argc * 2.22;
    volatile double d3 = argc * 3.33;
    
    // Vector types - use more registers
    v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    
    // Pointers to increase register pressure
    volatile int *p1 = &v1;
    volatile int *p2 = &v2;
    volatile float *pf1 = &f1;
    volatile float *pf2 = &f2;
    volatile double *pd1 = &d1;
    volatile double *pd2 = &d2;
    
    // Create data dependencies
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    
    // Loop with conditional control flow
    int iterations = (argc > 1) ? atoi(argv[1]) : 3;
    if (iterations > 10) iterations = 10; // Cap iterations
    
    for (int i = 0; i < iterations; i++) {
        // Complex conditional to create basic block boundaries
        if (i % 2 == 0) {
            // Perform computations before call
            v1 = v2 + v3 * i;
            v4 = v5 ^ v6;
            v7 = v8 << (i % 4);
            
            f1 = f2 * f3 + (float)i;
            f4 = f1 / f2 - f3;
            
            d1 = d2 * d3 / (i + 1);
            d2 = d1 + d3;
            
            // Vector operations
            vec1 = vec1 + vec2 * (float)i;
            vec2 = vec2 - vec1;
            dvec1 = dvec1 * dvec2;
            ivec1 = ivec1 + ivec2 * i;
            
            // Clobber specific registers before call
            asm volatile ("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                         "xmm0", "xmm1", "xmm2", "xmm3");
            
            // External function call - forces caller-save
            foo();
            
            // Clobber different registers after call
            asm volatile ("" ::: "memory", "rsi", "rdi", "r8", "r9",
                         "xmm4", "xmm5", "xmm6", "xmm7");
            
            // More computations to keep variables live
            v2 = v1 - v4;
            v3 = v5 | v6;
            v8 = v7 >> (i % 3);
            
            f2 = f3 * f4 - f1;
            f3 = f4 / f1 + f2;
            
            d3 = d1 - d2 * 0.5;
            d1 = d2 + d3;
            
            vec2 = vec1 * 2.0f - vec2;
            dvec2 = dvec1 + dvec2;
            ivec2 = ivec1 | ivec2;
            
        } else {
            // Alternative path with different operations
            v5 = v6 + v7 * i;
            v2 = v3 ^ v4;
            v1 = v8 << (i % 3);
            
            f3 = f4 * f1 + (float)i;
            f2 = f3 / f4 - f1;
            
            d2 = d3 * d1 / (i + 2);
            d3 = d2 + d1;
            
            // More vector operations
            vec1 = vec2 - vec1;
            dvec1 = dvec2 * 2.0;
            ivec1 = ivec2 - ivec1;
            
            // Clobber registers with different set
            asm volatile ("" ::: "memory", "r10", "r11", "r12", "r13",
                         "xmm8", "xmm9", "xmm10", "xmm11");
            
            // Different external call
            bar();
            
            // Clobber more registers
            asm volatile ("" ::: "memory", "r14", "r15", 
                         "xmm12", "xmm13", "xmm14", "xmm15");
            
            // Continue computations
            v6 = v5 - v2;
            v4 = v3 | v2;
            v7 = v1 >> (i % 2);
            
            f4 = f1 * f2 - f3;
            f1 = f2 / f3 + f4;
            
            d1 = d2 - d3 * 0.25;
            d2 = d3 + d1;
            
            vec1 = vec2 + vec1 * 0.5f;
            dvec2 = dvec1 - dvec2;
            ivec2 = ivec1 & ivec2;
            
            // Another call in the else branch
            if (i % 3 == 1) {
                asm volatile ("" ::: "memory", "rax", "rbx", "xmm0", "xmm1");
                baz();
                asm volatile ("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
            }
        }
        
        // Accumulate results to prevent optimization
        sum_int += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        sum_float += f1 + f2 + f3 + f4;
        sum_double += d1 + d2 + d3;
        
        // Vector accumulation
        for (int j = 0; j < 4; j++) {
            sum_float += vec1[j] + vec2[j];
            sum_int += ivec1[j] + ivec2[j];
        }
        for (int j = 0; j < 2; j++) {
            sum_double += dvec1[j] + dvec2[j];
        }
    }
    
    // Final computation and output to prevent dead code elimination
    double final_result = (double)sum_int + sum_float + sum_double;
    printf("Result: %f\n", final_result);
    
    // Use pointers to ensure they're live
    *p1 = sum_int;
    *pf1 = sum_float;
    *pd1 = sum_double;
    
    return (int)final_result % 256;
}
