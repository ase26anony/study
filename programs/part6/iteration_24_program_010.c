/* test_sched_context.c - Trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation to create scheduling regions */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Helper with memory dependencies */
static int helper_mem(int *arr, int idx) {
    int t1 = arr[idx];
    int t2 = arr[idx + 1];
    arr[idx] = t1 + t2;
    return t1 * t2;
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(volatile int seed) {
    int a = seed;
    int b = seed + 1;
    int c = seed + 2;
    int d = seed + 3;
    int e = seed + 4;
    
    /* Create many independent operations */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - c;
    int t5 = b * e;
    int t6 = t3 & t4;
    int t7 = t5 | t6;
    int t8 = t1 * t7;
    int t9 = t2 + t8;
    int t10 = t4 ^ t9;
    int t11 = t5 - t10;
    int t12 = t6 * t11;
    int t13 = t7 & t12;
    int t14 = t8 | t13;
    int t15 = t9 ^ t14;
    int t16 = t10 + t15;
    int t17 = t11 - t16;
    int t18 = t12 * t17;
    int t19 = t13 & t18;
    int t20 = t14 | t19;
    
    /* Add memory operations */
    volatile int mem[8];
    for (int i = 0; i < 8; i++) {
        mem[i] = seed + i;
    }
    
    int t21 = mem[0] + mem[1];
    int t22 = mem[2] * mem[3];
    int t23 = t20 + t21 * t22;
    
    return t23;
}

/* Function with vector intrinsics to trigger target-specific scheduling */
static __m128i vector_ops(volatile int seed1, volatile int seed2) {
    /* Create vector data */
    int data1[4] = {seed1, seed1 + 1, seed1 + 2, seed1 + 3};
    int data2[4] = {seed2, seed2 + 1, seed2 + 2, seed2 + 3};
    
    __m128i v1 = _mm_loadu_si128((__m128i*)data1);
    __m128i v2 = _mm_loadu_si128((__m128i*)data2);
    
    /* Multiple vector operations */
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_mullo_epi16(v1, v2);
    __m128i v5 = _mm_slli_epi32(v3, 2);
    __m128i v6 = _mm_srli_epi32(v4, 1);
    __m128i v7 = _mm_and_si128(v5, v6);
    __m128i v8 = _mm_or_si128(v7, v3);
    __m128i v9 = _mm_xor_si128(v8, v4);
    
    /* SSE floating point operations */
    __m128 f1 = _mm_set_ps(seed1 * 0.1f, seed1 * 0.2f, seed1 * 0.3f, seed1 * 0.4f);
    __m128 f2 = _mm_set_ps(seed2 * 0.5f, seed2 * 0.6f, seed2 * 0.7f, seed2 * 0.8f);
    __m128 f3 = _mm_add_ps(f1, f2);
    __m128 f4 = _mm_mul_ps(f3, f1);
    
    /* Mix integer and float results */
    __m128i vi = _mm_cvtps_epi32(f4);
    __m128i result = _mm_add_epi32(v9, vi);
    
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barriers(volatile int a, volatile int b) {
    int result;
    
    /* First computation */
    int t1 = a + b;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("# BEGIN BARRIER\n\t"
                  "nop\n\t"
                  "nop\n\t"
                  "# END BARRIER\n\t"
                  : : : "memory");
    
    /* Dependent computation */
    int t2 = t1 * a;
    
    /* Another barrier with register constraints */
    int t3;
    asm volatile ("movl %1, %%eax\n\t"
                  "imull %2, %%eax\n\t"
                  "movl %%eax, %0\n\t"
                  : "=r" (t3)
                  : "r" (t2), "r" (b)
                  : "%eax", "memory");
    
    /* More barriers with different constraints */
    int t4;
    asm volatile ("# COMPLEX BARRIER\n\t"
                  "addl %1, %2\n\t"
                  "movl %2, %0\n\t"
                  : "=r" (t4)
                  : "r" (t3), "r" (a)
                  : "cc", "memory");
    
    result = t4;
    
    /* Final barrier */
    asm volatile ("# FINAL BARRIER\n\t" : : : "memory");
    
    return result;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(volatile int seed, int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable condition */
        if ((seed + i) % 7 < 3) {
            /* Path with inline assembly */
            int temp;
            asm volatile ("movl %1, %%eax\n\t"
                         "addl $1, %%eax\n\t"
                         "movl %%eax, %0\n\t"
                         : "=r" (temp)
                         : "r" (seed)
                         : "%eax");
            total += temp * i;
        } else {
            /* Different computation path */
            total += helper_pure(seed, i);
        }
        
        /* Volatile memory access to prevent optimization */
        volatile int dummy = seed * i;
        (void)dummy;
    }
    
    return total;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 100; iter++) {
        volatile int loop_seed = seed1 + iter;
        
        /* Pattern 1: Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(loop_seed);
        
        /* Pattern 2: Vector operations for target-specific scheduling */
        __m128i vec_result = vector_ops(loop_seed, seed2 + iter);
        int vec_arr[4];
        _mm_storeu_si128((__m128i*)vec_arr, vec_result);
        total += vec_arr[0] + vec_arr[1] + vec_arr[2] + vec_arr[3];
        
        /* Pattern 3: Assembly barriers for state restoration */
        total += asm_barriers(loop_seed, seed3);
        
        /* Pattern 4: Unpredictable branching */
        total += branching_pattern(loop_seed, 10 + (iter % 5));
        
        /* Pattern 5: Memory-intensive operations */
        int mem_array[16];
        for (int i = 0; i < 16; i++) {
            mem_array[i] = loop_seed + i;
        }
        for (int i = 0; i < 8; i++) {
            total += helper_mem(mem_array, i * 2);
        }
        
        /* Mix in some pure function calls */
        total += helper_pure(total, loop_seed);
        
        /* Volatile write to prevent dead code elimination */
        volatile int checkpoint = total;
        (void)checkpoint;
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
