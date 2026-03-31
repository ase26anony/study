/* test_sched_context.c - Trigger free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling contexts */

/* Function with vector intrinsics to trigger target-specific scheduling */
static __m128 vector_intensive(volatile int a, volatile int b, volatile int c) {
    __m128 v1 = _mm_set1_ps((float)a);
    __m128 v2 = _mm_set1_ps((float)b);
    __m128 v3 = _mm_set1_ps((float)c);
    
    /* Chain of vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, _mm_set1_ps(1.0f));
    
    /* Inline assembly with SSE registers */
    asm volatile (
        "addps %1, %0\n\t"
        "mulps %2, %0\n\t"
        : "+x"(r4)
        : "x"(_mm_set1_ps(0.5f)), "x"(_mm_set1_ps(2.0f))
        : "memory"
    );
    
    return r4;
}

/* Dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(volatile int a, volatile int b, 
                           volatile int c, volatile int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Independent arithmetic operations */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = t1 & t2;
    t5 = t3 | t4;
    t6 = t5 << 2;
    t7 = t6 >> 1;
    t8 = t7 - t1;
    t9 = t8 * t2;
    t10 = t9 / (c + 1);
    t11 = t10 % (d + 1);
    t12 = t11 + t3;
    t13 = t12 - t4;
    t14 = t13 * t5;
    t15 = t14 & t6;
    t16 = t15 | t7;
    t17 = t16 ^ t8;
    t18 = t17 + t9;
    t19 = t18 - t10;
    t20 = t19 * t11;
    
    /* Memory operations to create dependencies */
    volatile int mem1 = t20;
    volatile int mem2 = t12;
    int t21 = mem1 + mem2;
    
    /* More operations */
    t21 += t13 * t14;
    t21 -= t15 / (t16 + 1);
    t21 ^= t17 & t18;
    t21 |= t19 << 3;
    
    return t21;
}

/* Function with artificial barriers for state restoration */
static int barrier_intensive(volatile int a, volatile int b, volatile int iter) {
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        int tmp1 = a * i;
        
        /* Artificial barrier - scheduler may try to move across this */
        asm volatile ("" : : : "memory");
        
        int tmp2 = b + i;
        
        /* Another barrier creating scheduling region */
        asm volatile ("# barrier %0" : : "r"(tmp1) : "memory");
        
        result += tmp1 * tmp2;
        
        /* Conditional with volatile to prevent optimization */
        if (*(volatile int*)&iter > 10) {
            asm volatile ("" : : : "memory");
            result -= tmp2;
        }
    }
    
    return result;
}

/* Pure function that acts as scheduling barrier */
static int pure_helper(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Function mixing pure calls and volatile for context creation */
static int mixed_calls(volatile int a, volatile int b, volatile int c) {
    int sum = 0;
    
    for (int i = 0; i < c; ++i) {
        /* Volatile read creates scheduling boundary */
        volatile int v1 = a + i;
        volatile int v2 = b - i;
        
        /* Pure function call - scheduler may save context around this */
        int r1 = pure_helper(v1, v2);
        
        /* Inline assembly with dependencies */
        int r2;
        asm volatile (
            "imull %1, %2\n\t"
            "addl %3, %2\n\t"
            : "=r"(r2)
            : "r"(v1), "r"(r1), "r"(i)
            : "cc"
        );
        
        sum += r2;
        
        /* Unpredictable branch */
        if (rand() % 100 > 50) {
            asm volatile ("" : : : "memory");
            sum -= pure_helper(v2, v1);
        }
    }
    
    return sum;
}

/* Main driver that creates multiple scheduling contexts */
int main(int argc, char **argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 98765;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        volatile int iter = (seed1 + outer) % 50 + 10;
        
        /* Call different patterns to trigger various scheduler behaviors */
        
        /* 1. Vector operations for target-specific scheduling */
        __m128 vec_result = vector_intensive(seed1 + outer, seed2, seed3);
        float f[4];
        _mm_store_ps(f, vec_result);
        total += (int)f[0] + (int)f[1] + (int)f[2] + (int)f[3];
        
        /* 2. Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1, seed2 + outer, seed3, seed4);
        
        /* 3. Barriers for state restoration */
        total += barrier_intensive(seed2, seed3, iter);
        
        /* 4. Mixed calls for context creation */
        total += mixed_calls(seed3, seed4, iter % 20 + 5);
        
        /* Unpredictable control flow */
        if (rand() % 10 == 0) {
            asm volatile ("" : : : "memory");
            total -= outer;
        }
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
