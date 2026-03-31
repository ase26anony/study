/* test_sched_context.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Volatile helper to prevent constant propagation */
static volatile int vol_var = 0;

/* Helper function with pure computation - scheduler may try to move it */
static int pure_helper(int a, int b) {
    return (a * b) + (a ^ b) - (a & b);
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a & c;
    int t5 = b | d;
    int t6 = t3 - t4;
    int t7 = t5 * t1;
    int t8 = t2 ^ t6;
    int t9 = t4 + t7;
    int t10 = t8 & t9;
    int t11 = t6 * t10;
    int t12 = t7 ^ t11;
    int t13 = t9 - t12;
    int t14 = t10 | t13;
    int t15 = t11 & t14;
    int t16 = t12 + t15;
    int t17 = t13 ^ t16;
    int t18 = t14 * t17;
    int t19 = t15 - t18;
    int t20 = t16 & t19;
    
    /* Add memory operations to create load/store dependencies */
    volatile int* mem = &vol_var;
    *mem = t20;
    int t21 = *mem + t17;
    
    return t21 + t18 + t19 + t20;
}

/* Function with SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Chain of SSE operations */
    __m128 res1 = _mm_add_ps(vec1, vec2);
    __m128 res2 = _mm_mul_ps(res1, vec3);
    __m128 res3 = _mm_sub_ps(res2, vec1);
    __m128 res4 = _mm_add_ps(res3, _mm_set1_ps(5.0f));
    
    /* Horizontal add pattern - creates dependencies */
    res4 = _mm_add_ps(res4, _mm_shuffle_ps(res4, res4, _MM_SHUFFLE(2, 3, 0, 1)));
    res4 = _mm_add_ps(res4, _mm_shuffle_ps(res4, res4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result[4] __attribute__((aligned(16)));
    _mm_store_ps(result, res4);
    
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers to force state save/restore */
static int asm_barrier_ops(int a, int b, int c) {
    int out1, out2, out3;
    
    /* First computation cluster */
    asm volatile ("addl %1, %0" : "=r"(out1) : "r"(a), "0"(b));
    
    /* Memory barrier - scheduler might try to move across this */
    asm volatile ("" ::: "memory");
    
    /* Second computation cluster with dependencies */
    asm volatile ("imull %1, %0" : "=r"(out2) : "r"(c), "0"(out1));
    
    /* Another barrier */
    asm volatile ("" ::: "memory");
    
    /* Third cluster */
    asm volatile ("xorl %1, %0" : "=r"(out3) : "r"(out2), "0"(a));
    
    return out3;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(int a, int b, int seed) {
    int result = a;
    
    /* Loop with volatile condition - scheduler can't predict trip count */
    for (int i = 0; i < (volatile int)seed; ++i) {
        /* Unpredictable branch */
        if (rand() % 2) {
            result += pure_helper(result, b);
        } else {
            result -= (result ^ b) * i;
        }
        
        /* Inline asm to prevent optimization */
        asm volatile ("" : "+r"(result));
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile seeds to prevent constant folding */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 456;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of context creation/freeing */
    for (int iter = 0; iter < 100; ++iter) {
        /* Mix different patterns in each iteration */
        
        /* 1. Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1 + iter, seed2 - iter, 
                                 seed3 * iter, seed4 ^ iter);
        
        /* 2. SSE operations for target-specific scheduling */
        total += (int)sse_operations((float)(seed1 + iter) / 10.0f,
                                    (float)(seed2 - iter) / 10.0f,
                                    (float)(seed3 * iter) / 10.0f,
                                    (float)(seed4 ^ iter) / 10.0f);
        
        /* 3. Assembly barrier operations */
        total += asm_barrier_ops(seed1 ^ iter, seed2 + iter, seed3 - iter);
        
        /* 4. Branching pattern with volatile loop */
        total += branching_pattern(seed4 + iter, seed1 - iter, 
                                  (iter % 10) + 1);
        
        /* 5. Pure helper called with volatile args */
        total += pure_helper(vol_var, iter);
        
        /* Occasionally add more complex patterns */
        if (iter % 7 == 0) {
            /* Nested loop to create scheduling region */
            for (int j = 0; j < 5; ++j) {
                total += dense_arithmetic(total, j, seed1, seed2);
            }
        }
        
        /* Modify volatile to create side effects */
        vol_var = total % 1000;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total % 255;
}
