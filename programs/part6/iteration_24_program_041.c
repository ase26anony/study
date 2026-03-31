/* test_sched_context.c - Trigger scheduler context allocation/freeing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper to prevent constant propagation */
static volatile int g_seed1, g_seed2, g_seed3;

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t1 & t2;
    int t5 = t3 | t4;
    int t6 = t2 - a;
    int t7 = t6 * t5;
    int t8 = t7 ^ t3;
    int t9 = t8 + t4;
    int t10 = t9 * 0x5A827999;
    int t11 = t10 ^ t7;
    int t12 = t11 + t6;
    int t13 = t12 * t8;
    int t14 = t13 ^ t10;
    int t15 = t14 + t9;
    int t16 = t15 * t11;
    int t17 = t16 ^ t13;
    int t18 = t17 + t12;
    int t19 = t18 * t14;
    int t20 = t19 ^ t16;
    int t21 = t20 + t15;
    int t22 = t21 * t17;
    int t23 = t22 ^ t19;
    int t24 = t23 + t18;
    int t25 = t24 * t20;
    int t26 = t25 ^ t22;
    int t27 = t26 + t21;
    int t28 = t27 * t23;
    int t29 = t28 ^ t25;
    int t30 = t29 + t24;
    return t30;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_sequence(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation with barrier */
    asm volatile ("addl %1, %0" : "=r"(res1) : "r"(a), "0"(b));
    
    /* Memory barrier that scheduler might try to cross */
    asm volatile ("" : : : "memory");
    
    /* Second computation */
    asm volatile ("imull %1, %0" : "=r"(res2) : "r"(c), "0"(res1));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Third computation with complex dependency */
    asm volatile ("xorl %1, %0\n\t"
                  "addl $0x9e3779b9, %0" 
                  : "=r"(res3) : "r"(res2), "0"(a));
    
    return res3;
}

/* Function with vector intrinsics to trigger target-specific scheduling */
static int vector_mix(int a, int b, int c, int d) {
    /* Create vector operations that engage SSE scheduling */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set1_epi32(0x9E3779B9);
    __m128i v3 = _mm_add_epi32(v1, v2);
    __m128i v4 = _mm_xor_si128(v1, v3);
    
    /* Mix with scalar operations */
    int s1 = a + b;
    int s2 = c * d;
    
    /* More vector ops */
    __m128 v5 = _mm_set_ps(s1, s2, a, b);
    __m128 v6 = _mm_set1_ps(3.14159f);
    __m128 v7 = _mm_add_ps(v5, v6);
    
    /* Extract results */
    int r[4];
    _mm_storeu_si128((__m128i*)r, v4);
    
    float f[4];
    _mm_storeu_ps(f, v7);
    
    return r[0] + r[1] + r[2] + r[3] + (int)f[0] + s1 + s2;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_sequence(int a, int b, int limit) {
    int result = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Volatile read makes branch unpredictable */
        int cond = g_seed1;
        
        if (cond & 1) {
            /* Path with inline asm and dependencies */
            int t1, t2;
            asm volatile ("movl %1, %0\n\t"
                          "rorl $7, %0" 
                          : "=r"(t1) : "r"(a));
            asm volatile ("addl %1, %0" : "+r"(t1) : "r"(b));
            result += t1;
        } else {
            /* Alternative path */
            int t2 = a * b + i;
            asm volatile ("" : : "r"(t2) : "memory");
            result ^= t2;
        }
        
        /* Mix in another volatile read */
        a ^= g_seed2;
    }
    
    return result;
}

/* Helper with function calls as scheduling barriers */
static int helper_func(int x, int y) {
    return x * y + 1;
}

static int function_barrier_sequence(int a, int b) {
    int sum = 0;
    volatile int iter = g_seed3 & 0xF;
    
    for (int i = 0; i < iter; ++i) {
        /* Function call acts as scheduling barrier */
        int tmp = helper_func(a + i, b - i);
        
        /* Inline asm with dependencies */
        int tmp2;
        asm volatile ("leal (%1,%2,2), %0" 
                      : "=r"(tmp2) : "r"(tmp), "r"(i));
        
        /* Memory operation */
        sum += tmp2;
        
        /* Modify inputs */
        a ^= sum;
        b += i;
    }
    
    return sum;
}

/* Main driver that combines all patterns */
int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant folding */
    g_seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    g_seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    g_seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Loop to increase chance of scheduler context operations */
    for (int outer = 0; outer < 100; ++outer) {
        /* Mix different patterns in each iteration */
        total += dense_arithmetic(g_seed1 + outer, 
                                 g_seed2 - outer, 
                                 g_seed3 ^ outer, 
                                 outer);
        
        total += asm_barrier_sequence(total, g_seed1, g_seed2);
        
        if (outer % 3 == 0) {
            total += vector_mix(total, g_seed2, g_seed3, outer);
        }
        
        total += branching_sequence(total, g_seed3, (g_seed1 & 7) + 2);
        
        total += function_barrier_sequence(total ^ g_seed1, 
                                          total + g_seed2);
        
        /* Modify seeds to change scheduling patterns */
        g_seed1 ^= total;
        g_seed2 += outer;
        g_seed3 = (g_seed3 << 3) | (g_seed3 >> 29);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
