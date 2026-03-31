/* test_sched_context.c
 * Designed to trigger free_sched_context logic in haifa-sched.cc
 * Compile with: gcc -O3 -funroll-loops -fschedule-insns2 -march=native -c test_sched_context.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics for x86 */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Helper functions to create scheduling complexity */

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t3 - e;
    int t5 = t4 * 7;
    int t6 = t5 & 0xFF;
    int t7 = t6 | a;
    int t8 = t7 << 3;
    int t9 = t8 / (b + 1);
    int t10 = t9 % 256;
    int t11 = t10 + c;
    int t12 = t11 * t11;
    int t13 = t12 - d;
    int t14 = t13 & e;
    int t15 = t14 | t1;
    int t16 = t15 ^ t2;
    int t17 = t16 + t3;
    int t18 = t17 * t4;
    int t19 = t18 / (t5 + 1);
    int t20 = t19 % 128;
    return t20;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_sequence(int a, int b, int c) {
    int result = 0;
    
    /* Create artificial dependencies with memory clobbers */
    asm volatile ("addl %1, %0" : "+r"(result) : "r"(a) : "memory");
    asm volatile ("subl %1, %0" : "+r"(result) : "r"(b) : "memory");
    asm volatile ("imull %1, %0" : "+r"(result) : "r"(c) : "memory");
    
    /* More operations with barriers */
    int temp = result;
    asm volatile ("" : "+r"(temp) : : "memory");
    result = temp * 2;
    
    asm volatile ("andl %1, %0" : "+r"(result) : "r"(a | b) : "memory");
    asm volatile ("xorl %1, %0" : "+r"(result) : "r"(c) : "memory");
    
    return result;
}

/* Function with SSE/vector operations for target-specific scheduling */
static __m128 vector_operations(__m128 a, __m128 b, __m128 c) {
    __m128 r1, r2, r3, r4, r5;
    
    /* Multiple dependent vector operations */
    r1 = _mm_add_ps(a, b);
    r2 = _mm_mul_ps(r1, c);
    r3 = _mm_sub_ps(r2, a);
    r4 = _mm_mul_ps(r3, b);
    r5 = _mm_add_ps(r4, c);
    
    /* More operations to increase scheduling complexity */
    r1 = _mm_add_ps(r5, r1);
    r2 = _mm_mul_ps(r1, _mm_set1_ps(2.0f));
    r3 = _mm_sub_ps(r2, _mm_set1_ps(1.0f));
    r4 = _mm_add_ps(r3, r5);
    
    /* Conditional-like operation using bitwise ops */
    __m128 mask = _mm_cmpgt_ps(r4, _mm_setzero_ps());
    r5 = _mm_or_ps(_mm_and_ps(mask, r4), 
                   _mm_andnot_ps(mask, _mm_set1_ps(0.5f)));
    
    return r5;
}

/* Function with mixed operations and unpredictable control flow */
static int mixed_control_flow(int a, int b, int c, volatile int* control) {
    int result = 0;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < *control; ++i) {
        /* Dense arithmetic inside loop */
        int t1 = a + i;
        int t2 = b * i;
        int t3 = t1 ^ t2;
        int t4 = t3 - c;
        
        /* Inline assembly with dependencies */
        asm volatile ("addl %1, %0" : "+r"(t4) : "r"(a) : "cc");
        
        /* Branch with unpredictable outcome */
        if (t4 & 1) {
            result += t4;
            asm volatile ("imull %1, %0" : "+r"(result) : "r"(b) : "memory");
        } else {
            result -= t4;
            asm volatile ("andl %1, %0" : "+r"(result) : "r"(c) : "memory");
        }
        
        /* Memory operations to create dependencies */
        volatile int mem_var = t4;
        result ^= mem_var;
    }
    
    return result;
}

/* Function with complex dependency chain */
static int complex_dependency_chain(int start, int iterations) {
    int a = start, b = start + 1, c = start + 2;
    int result = 0;
    
    for (int i = 0; i < iterations; ++i) {
        /* Long chain of dependent operations */
        int r1 = a + b;
        asm volatile ("" : "+r"(r1) : : "memory");
        int r2 = r1 * c;
        asm volatile ("" : "+r"(r2) : : "memory");
        int r3 = r2 ^ a;
        asm volatile ("" : "+r"(r3) : : "memory");
        int r4 = r3 - b;
        asm volatile ("" : "+r"(r4) : : "memory");
        int r5 = r4 & c;
        asm volatile ("" : "+r"(r5) : : "memory");
        int r6 = r5 | r1;
        asm volatile ("" : "+r"(r6) : : "memory");
        int r7 = r6 << 2;
        asm volatile ("" : "+r"(r7) : : "memory");
        int r8 = r7 / (c + 1);
        asm volatile ("" : "+r"(r8) : : "memory");
        
        result += r8;
        
        /* Update values for next iteration */
        a = r3;
        b = r6;
        c = r8;
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(int argc, char** argv) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int control = argc > 4 ? atoi(argv[4]) : 10;
    
    int total = 0;
    
    /* Loop to increase chance of scheduling context creation/freeing */
    for (int outer = 0; outer < 100; ++outer) {
        /* Pattern 1: Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1 + outer, 
                                 seed2 + outer * 2,
                                 seed3, 
                                 seed1 ^ seed2,
                                 seed2 ^ seed3);
        
        /* Pattern 2: Assembly barriers for state restoration */
        total += asm_barrier_sequence(total, seed1, seed2);
        
        /* Pattern 3: Vector operations for target-specific scheduling */
        __m128 vec_a = _mm_setr_ps(seed1 * 0.1f, seed2 * 0.2f, 
                                  seed3 * 0.3f, total * 0.01f);
        __m128 vec_b = _mm_setr_ps(seed2 * 0.4f, seed3 * 0.5f,
                                  seed1 * 0.6f, outer * 0.1f);
        __m128 vec_c = _mm_setr_ps(seed3 * 0.7f, seed1 * 0.8f,
                                  seed2 * 0.9f, 1.0f);
        
        __m128 vec_result = vector_operations(vec_a, vec_b, vec_c);
        float vec_sum[4];
        _mm_storeu_ps(vec_sum, vec_result);
        total += (int)(vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3]);
        
        /* Pattern 4: Mixed control flow with volatile access */
        total += mixed_control_flow(seed1, seed2, seed3, &control);
        
        /* Pattern 5: Complex dependency chain */
        total += complex_dependency_chain(total % 100, control % 5 + 3);
        
        /* Modify control variable to change loop behavior */
        control = (control * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
