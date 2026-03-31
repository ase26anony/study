/* test_sched_context.c - Comprehensive test for GCC Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation to encourage scheduling */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (b & c);
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - c;
    int t5 = b | d;
    int t6 = t3 & t4;
    int t7 = t5 << 2;
    int t8 = t6 * t7;
    int t9 = t8 >> 1;
    int t10 = t9 + a;
    int t11 = t10 - b;
    int t12 = t11 * c;
    int t13 = t12 / (d + 1);
    int t14 = t13 ^ t1;
    int t15 = t14 | t2;
    int t16 = t15 & t3;
    int t17 = t16 + t4;
    int t18 = t17 - t5;
    int t19 = t18 * t6;
    int t20 = t19 >> 3;
    int t21 = t20 ^ t7;
    int t22 = t21 | t8;
    int t23 = t22 & t9;
    int t24 = t23 + t10;
    int t25 = t24 - t11;
    return t25;
}

/* Function with vector operations to trigger target-specific scheduling */
static __m128 vector_operations(__m128 a, __m128 b, __m128 c) {
    __m128 r1 = _mm_add_ps(a, b);
    __m128 r2 = _mm_mul_ps(r1, c);
    __m128 r3 = _mm_sub_ps(r2, a);
    __m128 r4 = _mm_add_ps(r3, b);
    __m128 r5 = _mm_mul_ps(r4, _mm_set1_ps(2.0f));
    __m128 r6 = _mm_add_ps(r5, _mm_set1_ps(1.0f));
    __m128 r7 = _mm_sub_ps(r6, _mm_set1_ps(0.5f));
    return _mm_mul_ps(r7, _mm_set1_ps(1.5f));
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_test(int a, int b, int c, volatile int* mem) {
    int result;
    
    /* First computation block */
    int t1 = a * b + c;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    int t2 = t1 - (a ^ b);
    
    /* Another barrier */
    asm volatile ("# This is a comment barrier" : : : "memory", "eax", "ebx");
    
    /* More computation with memory access */
    *mem = t2;
    int t3 = *mem + c;
    
    /* Final barrier with specific register clobber */
    asm volatile ("movl %1, %%eax\n\t"
                  "addl %2, %%eax\n\t"
                  "movl %%eax, %0"
                  : "=r"(result)
                  : "r"(t3), "r"(a)
                  : "eax", "memory");
    
    return result;
}

/* Function with mixed operations and unpredictable branching */
static int branching_test(int a, int b, volatile int* cond) {
    int result = 0;
    
    /* Create multiple independent computations */
    int x1 = a + b;
    int x2 = a * b;
    int x3 = a ^ b;
    int x4 = a - b;
    int x5 = a | b;
    
    /* Unpredictable branch to encourage speculative scheduling */
    if (*cond & 1) {
        result += x1 * x2;
        /* Insert barrier in one path */
        asm volatile ("" : : : "memory");
    } else {
        result += x3 - x4;
    }
    
    /* Another level of branching */
    if (*cond & 2) {
        result += x5 << 2;
    } else {
        result += x1 >> 1;
    }
    
    /* More computations after branches */
    int y1 = result * a;
    int y2 = result + b;
    int y3 = y1 ^ y2;
    
    return y3;
}

/* Function with loop containing scheduling complexity */
static int loop_scheduling_test(int start, volatile int limit) {
    int acc = start;
    
    /* Loop with volatile limit to prevent unrolling */
    for (int i = 0; i < limit; ++i) {
        /* Mix of operations inside loop */
        int t1 = acc + i;
        int t2 = acc * i;
        int t3 = t1 ^ t2;
        
        /* Inline assembly with dependencies */
        int t4;
        asm volatile ("addl %1, %0\n\t"
                      "imull %2, %0"
                      : "=r"(t4)
                      : "r"(t3), "r"(i)
                      : "cc");
        
        /* Memory operation */
        volatile int mem = t4;
        acc = mem + (i & 0xFF);
        
        /* Conditional with pure function call */
        if (i & 1) {
            acc = pure_helper(acc, i, t4);
        }
    }
    
    return acc;
}

/* Main function that orchestrates all tests */
int main(int argc, char** argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 456;
    
    int total = 0;
    volatile int mem = 0;
    
    /* Multiple iterations to increase chance of scheduler context creation */
    for (int iter = 0; iter < 100; ++iter) {
        /* Vary the conditions to create different scheduling scenarios */
        volatile int cond = seed1 + iter;
        
        /* Test 1: Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1 + iter, seed2, seed3, seed4);
        
        /* Test 2: Vector operations for target-specific scheduling */
        __m128 vec_a = _mm_set_ps(seed1 * 0.1f, seed2 * 0.2f, 
                                 seed3 * 0.3f, seed4 * 0.4f);
        __m128 vec_b = _mm_set_ps(iter * 0.5f, (iter+1) * 0.6f,
                                 (iter+2) * 0.7f, (iter+3) * 0.8f);
        __m128 vec_c = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 vec_result = vector_operations(vec_a, vec_b, vec_c);
        
        /* Extract result from vector */
        float vec_floats[4];
        _mm_store_ps(vec_floats, vec_result);
        total += (int)(vec_floats[0] + vec_floats[1] + 
                      vec_floats[2] + vec_floats[3]);
        
        /* Test 3: Assembly barrier test */
        total += asm_barrier_test(seed1, seed2 + iter, seed3, &mem);
        
        /* Test 4: Branching test with unpredictable conditions */
        total += branching_test(seed3, seed4, &cond);
        
        /* Test 5: Loop scheduling test with volatile limit */
        volatile int limit = 10 + (iter % 5);
        total += loop_scheduling_test(total, limit);
        
        /* Mix in some pure function calls */
        total = pure_helper(total, iter, seed1);
    }
    
    /* Ensure result is used */
    printf("Final result: %d\n", total);
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
