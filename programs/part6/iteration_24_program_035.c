/* test_sched_context.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
volatile int seed1, seed2, seed3;

/* Helper function with pure computation - scheduler may try to move it */
static int helper_pure(int a, int b) {
    return (a * b) + (a ^ b) - (a & b);
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a & b;
    int t5 = c | d;
    int t6 = t3 - t4;
    int t7 = t5 << 2;
    int t8 = t6 * t7;
    int t9 = t8 >> 1;
    int t10 = t9 + a;
    int t11 = t10 - b;
    int t12 = t11 * c;
    int t13 = t12 / (d ? d : 1);
    int t14 = t13 ^ t8;
    int t15 = t14 & t9;
    int t16 = t15 | t10;
    int t17 = t16 << 3;
    int t18 = t17 >> 2;
    int t19 = t18 + t11;
    int t20 = t19 - t12;
    int t21 = t20 * t13;
    int t22 = t21 / (t14 ? t14 : 1);
    int t23 = t22 ^ t15;
    int t24 = t23 & t16;
    int t25 = t24 | t17;
    return t25;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barriers(int a, int b) {
    int result = 0;
    
    /* First computation with memory clobber */
    asm volatile ("addl %1, %0" 
                  : "+r" (result) 
                  : "r" (a)
                  : "memory");
    
    /* Artificial barrier - scheduler might try to move across this */
    asm volatile ("# OPAQUE BARRIER" ::: "memory");
    
    /* Second computation */
    asm volatile ("imull %1, %0" 
                  : "+r" (result) 
                  : "r" (b));
    
    /* Another barrier */
    asm volatile ("# ANOTHER BARRIER" ::: "memory");
    
    /* Third computation with dependency */
    int temp = b * 2;
    asm volatile ("addl %1, %0" 
                  : "+r" (result) 
                  : "r" (temp));
    
    return result;
}

/* Function with SSE/MMX intrinsics to trigger target-specific scheduling */
static __m128i vector_computation(__m128i a, __m128i b) {
    __m128i r1 = _mm_add_epi32(a, b);
    __m128i r2 = _mm_mullo_epi16(a, b);
    __m128i r3 = _mm_slli_epi32(r1, 2);
    __m128i r4 = _mm_srli_epi32(r2, 1);
    __m128i r5 = _mm_add_epi32(r3, r4);
    __m128i r6 = _mm_sub_epi32(r5, a);
    __m128i r7 = _mm_mulhi_epi16(r6, b);
    return _mm_add_epi32(r7, r5);
}

/* Function with branching and unpredictable control flow */
static int branching_pattern(int a, int b, int iter) {
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Volatile read to prevent optimization */
        volatile int cond = seed3;
        
        if (cond & 1) {
            result += helper_pure(a + i, b);
        } else {
            result -= helper_pure(b, a - i);
        }
        
        /* Mix in some inline assembly */
        asm volatile ("# LOOP BODY" ::: "memory");
        
        /* Unpredictable branch */
        if (i % 3 == 0) {
            result ^= (a * b);
        } else if (i % 3 == 1) {
            result |= (a ^ b);
        } else {
            result &= (a | b);
        }
    }
    
    return result;
}

/* Complex function mixing all patterns */
static int mixed_scheduling(int a, int b, int c, int d, int iter) {
    int total = 0;
    
    /* Start with dense arithmetic */
    total += dense_arithmetic(a, b, c, d);
    
    /* Add vector computations if supported */
    __m128i vec_a = _mm_set_epi32(a, b, c, d);
    __m128i vec_b = _mm_set_epi32(b, c, d, a);
    __m128i vec_result = vector_computation(vec_a, vec_b);
    
    /* Extract results from vector */
    int vec_results[4];
    _mm_storeu_si128((__m128i*)vec_results, vec_result);
    total += vec_results[0] + vec_results[1] + vec_results[2] + vec_results[3];
    
    /* Add branching pattern */
    total += branching_pattern(a, b, iter % 10);
    
    /* Add assembly barriers */
    total += asm_barriers(c, d);
    
    /* More dense computations */
    for (int i = 0; i < 5; ++i) {
        total += dense_arithmetic(total, a + i, b + i, c + i);
    }
    
    return total;
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from command line to prevent constant folding */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the iteration count to create different scheduling contexts */
        volatile int iter_mod = seed1 + outer;
        
        /* Call different scheduling patterns in sequence */
        total += mixed_scheduling(
            seed1 + outer, 
            seed2 - outer, 
            seed3 ^ outer, 
            seed1 * outer, 
            iter_mod
        );
        
        /* Additional independent computations */
        for (int inner = 0; inner < 10; ++inner) {
            /* Create memory dependencies */
            int *ptr = malloc(sizeof(int) * 10);
            if (ptr) {
                for (int j = 0; j < 10; ++j) {
                    ptr[j] = dense_arithmetic(seed1, seed2, j, inner);
                }
                
                /* Compute with memory results */
                int sum = 0;
                for (int j = 0; j < 10; ++j) {
                    sum += ptr[j];
                }
                
                total += asm_barriers(sum, inner);
                free(ptr);
            }
        }
        
        /* More vector computations */
        __m128i v1 = _mm_set_epi32(seed1, seed2, seed3, outer);
        __m128i v2 = _mm_set1_epi32(seed1 ^ outer);
        __m128i vres = vector_computation(v1, v2);
        
        int varr[4];
        _mm_storeu_si128((__m128i*)varr, vres);
        total += varr[0] + varr[1] + varr[2] + varr[3];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
