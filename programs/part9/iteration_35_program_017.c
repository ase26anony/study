#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    
    /* 
     * Declare MANY local variables of mixed types to maximize register pressure
     * Mark key ones as volatile to prevent optimization
     */
    volatile int vi1 = 1, vi2 = 2, vi3 = 3, vi4 = 4, vi5 = 5;
    int vi6 = 6, vi7 = 7, vi8 = 8, vi9 = 9, vi10 = 10;
    volatile float vf1 = 1.1f, vf2 = 2.2f, vf3 = 3.3f, vf4 = 4.4f;
    float vf5 = 5.5f, vf6 = 6.6f, vf7 = 7.7f, vf8 = 8.8f;
    volatile double vd1 = 1.11, vd2 = 2.22, vd3 = 3.33, vd4 = 4.44;
    double vd5 = 5.55, vd6 = 6.66, vd7 = 7.77, vd8 = 8.88;
    
    /* Pointer variables */
    volatile int *p1 = &vi1;
    int *p2 = &vi2, *p3 = &vi3, *p4 = &vi4, *p5 = &vi5;
    
    /* Vector variables - these use SSE/AVX registers */
    volatile v4sf vec1 = {1.0f, 2.0f, 3.0f, 4.0f};
    v4sf vec2 = {5.0f, 6.0f, 7.0f, 8.0f};
    v4sf vec3 = {9.0f, 10.0f, 11.0f, 12.0f};
    volatile v2df dvec1 = {1.0, 2.0};
    v2df dvec2 = {3.0, 4.0};
    v2df dvec3 = {5.0, 6.0};
    volatile v4si ivec1 = {1, 2, 3, 4};
    v4si ivec2 = {5, 6, 7, 8};
    v4si ivec3 = {9, 10, 11, 12};
    
    /* Additional variables for more pressure */
    long vl1 = 100, vl2 = 200, vl3 = 300, vl4 = 400;
    volatile long vl5 = 500;
    short vs1 = 10, vs2 = 20, vs3 = 30, vs4 = 40;
    volatile short vs5 = 50;
    
    /* Result accumulator */
    double total_result = 0.0;
    
    /* 
     * Loop with complex control flow to create basic block boundaries
     * where save/restore instructions might be reordered
     */
    for (int i = 0; i < iterations; i++) {
        /* 
         * Phase 1: Intensive computation using all variables
         * Creates data dependencies to keep variables live
         */
        vi1 = vi2 * vi3 + i;
        vi2 = vi4 ^ vi5;
        vi3 = vi6 & vi7;
        vi4 = vi8 | vi9;
        vi5 = vi10 << 2;
        
        vf1 = vf2 * vf3 + i * 0.1f;
        vf2 = vf4 / (vf5 + 0.001f);
        vf3 = vf6 - vf7;
        vf4 = vf8 * 2.0f;
        
        vd1 = vd2 + vd3 * i;
        vd2 = vd4 - vd5;
        vd3 = vd6 * vd7;
        vd4 = vd8 / 2.0;
        
        /* Vector operations - use SSE/AVX registers */
        vec1 = vec1 + vec2 * (float)i;
        vec2 = vec3 - vec1;
        vec3 = vec1 * vec2;
        
        dvec1 = dvec1 + dvec2;
        dvec2 = dvec3 - dvec1;
        dvec3 = dvec1 * dvec2;
        
        ivec1 = ivec1 + ivec2 * i;
        ivec2 = ivec3 - ivec1;
        ivec3 = ivec1 & ivec2;
        
        /* Pointer arithmetic */
        p2 = p1 + 1;
        p3 = p2 + vi1;
        p4 = p3 - 2;
        p5 = p4 + vi2;
        
        /* 
         * CRITICAL SECTION: Insert asm clobber before call
         * This forces the compiler to save registers
         */
        asm volatile("" ::: "memory", "rax", "rbx", "rcx", "rdx", 
                     "xmm0", "xmm1", "xmm2", "xmm3");
        
        /* 
         * External function call - cannot be inlined
         * Forces caller-save register preservation
         */
        clobber_func1();
        
        /* 
         * More asm clobbering with DIFFERENT registers
         * Forces additional save/restore sequences
         */
        asm volatile("" ::: "memory", "rsi", "rdi", "r8", "r9",
                     "xmm4", "xmm5", "xmm6", "xmm7");
        
        /* Phase 2: More computations keeping variables live */
        vi6 = vi1 + vi2 - vi3;
        vi7 = vi4 * vi5 / (vi6 + 1);
        vi8 = vi9 ^ vi10;
        vi9 = vi7 & vi8;
        vi10 = vi6 | vi9;
        
        vf5 = vf1 + vf2 - vf3;
        vf6 = vf4 * vf5;
        vf7 = vf6 / (vf8 + 0.001f);
        vf8 = vf7 - vf1;
        
        vd5 = vd1 + vd2 - vd3;
        vd6 = vd4 * vd5;
        vd7 = vd6 / (vd8 + 0.001);
        vd8 = vd7 - vd1;
        
        /* More vector ops */
        vec1 = vec2 + vec3;
        vec2 = vec1 * 2.0f;
        vec3 = vec2 - vec1;
        
        /* 
         * Another asm clobber with mixed register types
         * This creates complex save/restore patterns
         */
        asm volatile("" ::: "memory", "r10", "r11", "r12", "r13", "r14", "r15",
                     "xmm8", "xmm9", "xmm10", "xmm11", "xmm12", "xmm13");
        
        /* Second external call */
        clobber_func2();
        
        /* Conditional block to create more basic block complexity */
        if (i % 2 == 0) {
            /* Even iteration: different computation path */
            asm volatile("" ::: "memory", "rax", "rbx", "xmm0", "xmm1", "xmm14", "xmm15");
            
            vec1 = vec1 * 3.0f;
            vec2 = vec2 / 2.0f;
            dvec1 = dvec1 + 1.0;
            dvec2 = dvec2 - 0.5;
            
            clobber_func3();
            
            asm volatile("" ::: "memory", "rcx", "rdx", "xmm2", "xmm3");
        } else {
            /* Odd iteration: alternative path */
            asm volatile("" ::: "memory", "rsi", "rdi", "xmm4", "xmm5");
            
            vec3 = vec3 * 1.5f;
            dvec3 = dvec3 / 3.0;
            
            /* Nested conditional for more block complexity */
            if (vi1 > 100) {
                asm volatile("" ::: "memory", "r8", "r9", "xmm6", "xmm7");
                clobber_func1();
            }
        }
        
        /* Final computations in the loop */
        vi1 = vi1 + 1;
        vi2 = vi2 - 1;
        vf1 = vf1 * 1.1f;
        vf2 = vf2 / 1.1f;
        vd1 = vd1 + 0.1;
        vd2 = vd2 - 0.1;
        
        /* Accumulate results to prevent dead code elimination */
        total_result += (double)vi1 + (double)vi2 + (double)vf1 + (double)vf2 + vd1 + vd2;
        total_result += vec1[0] + vec2[1] + vec3[2];
        total_result += dvec1[0] + dvec2[1];
    }
    
    /* 
     * Final aggregation and output to prevent optimization
     * Mix all variable types in final computation
     */
    double final_result = total_result;
    final_result += (double)vi3 + (double)vi4 + (double)vi5;
    final_result += (double)vf3 + (double)vf4 + (double)vf5;
    final_result += vd3 + vd4 + vd5;
    final_result += vec1[3] + vec2[0] + vec3[1];
    final_result += dvec3[0] + dvec3[1];
    
    /* Use pointers to create memory dependencies */
    *p1 = vi1;
    vi2 = *p2;
    *p3 = vi3;
    vi4 = *p4;
    
    /* Print result to create observable side effect */
    printf("Result: %f\n", final_result);
    
    return (int)final_result % 256;
}
