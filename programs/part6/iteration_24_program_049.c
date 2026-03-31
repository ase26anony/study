/* test_sched_context.c - Test program to trigger Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile variables to prevent optimization */
volatile int g_volatile_seed1, g_volatile_seed2, g_volatile_seed3;
volatile int g_volatile_iter;

/* Helper function with scheduling barriers */
static int __attribute__((noinline)) 
helper_with_barriers(int a, int b, int c) {
    int t1, t2, t3, t4, t5;
    
    /* Create artificial scheduling barrier */
    asm volatile ("" : : : "memory");
    
    /* Chain of dependent operations */
    t1 = a * b + c;
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    t2 = t1 ^ (a << 3);
    t3 = t2 * b - c;
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    t4 = t3 & 0xFFFF;
    t5 = t4 | (b & 0xFF);
    
    return t5;
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int __attribute__((noinline))
dense_arithmetic(int a, int b, int c, int d) {
    /* Create many independent operations */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - d;
    int t5 = b << 2;
    int t6 = c >> 1;
    int t7 = t3 & t4;
    int t8 = t5 | t6;
    int t9 = t7 * t8;
    int t10 = t9 + a;
    int t11 = t10 - b;
    int t12 = t11 * c;
    int t13 = t12 / (d + 1);
    int t14 = t13 ^ a;
    int t15 = t14 & b;
    int t16 = t15 | c;
    int t17 = t16 << 3;
    int t18 = t17 >> 1;
    int t19 = t18 + d;
    int t20 = t19 * 7;
    int t21 = t20 - 13;
    int t22 = t21 & 0xFF;
    int t23 = t22 | 0x80;
    int t24 = t23 ^ 0x55;
    int t25 = t24 * 3;
    int t26 = t25 + 17;
    int t27 = t26 - 5;
    int t28 = t27 * 2;
    int t29 = t28 / 4;
    int t30 = t29 + 1;
    
    /* Mix with memory operations */
    volatile int mem_var = t30;
    int t31 = mem_var * 2;
    
    return t31;
}

/* Function using SSE/MMX intrinsics to trigger target-specific scheduling */
static int __attribute__((noinline))
vector_operations(int a, int b, int c, int d) {
    /* Create vector data */
    __m128i vec1 = _mm_set_epi32(a, b, c, d);
    __m128i vec2 = _mm_set_epi32(d, c, b, a);
    __m128i vec3 = _mm_set1_epi32(0x12345678);
    
    /* Vector operations that engage SSE scheduling */
    __m128i result1 = _mm_add_epi32(vec1, vec2);
    __m128i result2 = _mm_sub_epi32(result1, vec3);
    __m128i result3 = _mm_and_si128(result2, vec1);
    __m128i result4 = _mm_or_si128(result3, vec2);
    
    /* Extract results */
    int res[4];
    _mm_storeu_si128((__m128i*)res, result4);
    
    /* Scalar operations mixed with vector results */
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum += res[i];
        sum ^= (res[i] << (i * 3));
    }
    
    return sum;
}

/* Function with unpredictable branching for speculative scheduling */
static int __attribute__((noinline))
branchy_computation(int a, int b, int limit) {
    int result = 0;
    
    /* Loop with volatile condition to prevent prediction */
    for (int i = 0; i < limit; i++) {
        /* Unpredictable branch */
        if ((a ^ i) & 1) {
            result += helper_with_barriers(a, b, i);
        } else {
            result -= dense_arithmetic(a, b, i, result);
        }
        
        /* Another unpredictable condition */
        volatile int cond = (i * a) & 0xF;
        if (cond > 8) {
            result ^= vector_operations(a, b, i, result);
        }
        
        /* Modify variables to create dependencies */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = (b * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Complex loop structure to create multiple scheduling contexts */
static int __attribute__((noinline))
complex_scheduling_test(int seed1, int seed2, int seed3, int iterations) {
    int total = 0;
    
    /* Outer loop with volatile iteration count */
    for (volatile int outer = 0; outer < iterations; outer++) {
        int local_seed1 = seed1 + outer;
        int local_seed2 = seed2 ^ outer;
        int local_seed3 = seed3 * (outer + 1);
        
        /* First scheduling region: dense arithmetic */
        total += dense_arithmetic(local_seed1, local_seed2, 
                                 local_seed3, total);
        
        /* Second scheduling region: vector operations */
        total ^= vector_operations(local_seed2, local_seed3,
                                  total, local_seed1);
        
        /* Third scheduling region: branchy code */
        int branch_limit = (local_seed1 & 0x7) + 3;
        total += branchy_computation(local_seed1, local_seed2, branch_limit);
        
        /* Fourth scheduling region: mixed operations with barriers */
        for (int inner = 0; inner < 5; inner++) {
            /* Create inline assembly with dependencies */
            int t1, t2, t3;
            asm volatile (
                "movl %1, %0\n\t"
                "addl %2, %0\n\t"
                "imull %3, %0"
                : "=r"(t1)
                : "r"(local_seed1), "r"(inner), "r"(total)
                : "cc"
            );
            
            asm volatile (
                "xorl %1, %0\n\t"
                "roll $3, %0"
                : "+r"(t1)
                : "r"(local_seed2)
                : "cc"
            );
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
            
            t2 = t1 & 0xFFF;
            t3 = t2 | (local_seed3 & 0xFF);
            
            total += t3;
        }
        
        /* Modify seeds for next iteration */
        seed1 = (seed1 * 3 + 1) & 0xFFF;
        seed2 = (seed2 * 5 + 2) & 0xFFF;
        seed3 = (seed3 * 7 + 3) & 0xFFF;
    }
    
    return total;
}

/* Main function with volatile initialization */
int main(int argc, char *argv[]) {
    /* Initialize volatile seeds from command line */
    g_volatile_seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    g_volatile_seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    g_volatile_seed3 = (argc > 3) ? atoi(argv[3]) : 13579;
    g_volatile_iter = (argc > 4) ? atoi(argv[4]) : 100;
    
    int seed1 = g_volatile_seed1;
    int seed2 = g_volatile_seed2;
    int seed3 = g_volatile_seed3;
    int iterations = g_volatile_iter;
    
    /* Perform the complex scheduling test */
    int result = complex_scheduling_test(seed1, seed2, seed3, iterations);
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    return result & 0xFF;
}
