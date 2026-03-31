/* test_sched_context.c - Trigger scheduler context allocation/deallocation */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile helpers to prevent optimization */
static volatile int v_counter = 0;
static volatile int v_seed1, v_seed2, v_seed3;

/* Helper with scheduling barriers */
static int __attribute__((noinline)) 
helper_with_barrier(int a, int b) {
    int result;
    /* Create artificial scheduling barrier */
    asm volatile ("# BEGIN BARRIER" ::: "memory");
    result = a * b + (a ^ b);
    asm volatile ("# END BARRIER" ::: "memory");
    return result;
}

/* Dense arithmetic sequence to fill instruction queue */
static int __attribute__((noinline))
dense_arithmetic(int a, int b, int c, int d) {
    /* Create many independent operations */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - c;
    int t5 = b << 2;
    int t6 = d >> 1;
    int t7 = t3 & t4;
    int t8 = t5 | t6;
    int t9 = t7 * t8;
    int t10 = t1 + t2 + t3;
    int t11 = t4 - t5 * t6;
    int t12 = t7 ^ t8 ^ t9;
    int t13 = t10 & t11 | t12;
    int t14 = t13 * a + b;
    int t15 = t14 * c - d;
    int t16 = t15 ^ t13 ^ t14;
    int t17 = t16 << 3;
    int t18 = t17 >> 1;
    int t19 = t18 + t9 * t10;
    int t20 = t11 - t12 * t13;
    
    /* Mix with memory operations */
    volatile int mem_var = t19;
    int t21 = mem_var + t20;
    volatile int mem_var2 = t21;
    int t22 = mem_var2 * t14;
    
    return t22 + t15 + t16;
}

/* Vector operations to trigger target-specific scheduling */
static int __attribute__((noinline))
vector_operations(int a, int b, int c, int d) {
#ifdef __SSE2__
    /* Create vector operations that engage SSE scheduling */
    __m128i v1 = _mm_set_epi32(a, b, c, d);
    __m128i v2 = _mm_set_epi32(b, c, d, a);
    __m128i v3 = _mm_set_epi32(c, d, a, b);
    
    /* Chain multiple vector operations */
    __m128i r1 = _mm_add_epi32(v1, v2);
    __m128i r2 = _mm_mullo_epi16(v1, v3);  /* Note: requires SSE4.1 */
    __m128i r3 = _mm_slli_epi32(r1, 2);
    __m128i r4 = _mm_srli_epi32(r2, 1);
    __m128i r5 = _mm_add_epi32(r3, r4);
    
    /* Extract results */
    int results[4];
    _mm_storeu_si128((__m128i*)results, r5);
    
    /* Create dependencies between vector and scalar ops */
    int scalar = a + b + c + d;
    for (int i = 0; i < 4; i++) {
        scalar ^= results[i];
    }
    
    /* More vector operations with scheduling barriers */
    asm volatile ("# VECTOR BARRIER" ::: "memory");
    __m128 vf1 = _mm_set_ps(a, b, c, d);
    __m128 vf2 = _mm_set_ps(b, c, d, a);
    __m128 vf3 = _mm_add_ps(vf1, vf2);
    
    float fresults[4];
    _mm_storeu_ps(fresults, vf3);
    
    for (int i = 0; i < 4; i++) {
        scalar += (int)fresults[i];
    }
    
    return scalar;
#else
    /* Fallback for non-SSE2 targets */
    return a * b + c * d;
#endif
}

/* Complex loop with unpredictable branching */
static int __attribute__((noinline))
branching_pattern(int iterations, int seed) {
    int result = seed;
    volatile int mod = seed % 7;  /* Prevent prediction */
    
    for (int i = 0; i < iterations; i++) {
        /* Unpredictable branch */
        if ((i + mod) % 3 == 0) {
            /* Branch 1: Mix of operations */
            result += helper_with_barrier(result, i);
            result ^= (i * 0x5A827999);
        } else if ((i + mod) % 3 == 1) {
            /* Branch 2: Different operation mix */
            result -= dense_arithmetic(result, i, seed, mod);
            result |= (i << 3);
        } else {
            /* Branch 3: Vector operations when available */
            result = vector_operations(result, i, seed, mod) & 0x7FFFFFFF;
        }
        
        /* Insert scheduling barrier occasionally */
        if (i % 5 == 0) {
            asm volatile ("# LOOP BARRIER %0" : "+r"(result) :: "memory");
        }
        
        /* Volatile update to prevent loop optimization */
        v_counter = i;
    }
    
    return result;
}

/* Function with inline assembly constraints */
static int __attribute__((noinline))
inline_asm_constraints(int a, int b, int c) {
    int out1, out2, out3;
    
    /* Multiple asm statements with dependencies */
    asm volatile (
        "add %0, %1, %2\n\t"
        "mul %3, %0, %1\n\t"
        : "=r"(out1), "=r"(out2)
        : "r"(a), "r"(b), "1"(c)
        : "cc"
    );
    
    asm volatile (
        "eor %0, %1, %2\n\t"
        "add %0, %0, #1\n\t"
        : "=r"(out3)
        : "r"(out1), "r"(out2)
        : "cc"
    );
    
    /* Memory clobber to create scheduling barrier */
    asm volatile ("" ::: "memory");
    
    /* More operations after barrier */
    int temp = out3;
    for (int i = 0; i < 3; i++) {
        temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return temp;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Initialize volatile seeds from command line to prevent constant propagation */
    v_seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    v_seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    v_seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    int total = 0;
    volatile int iter_count = 100;  /* Prevent unrolling */
    
    /* Main loop that exercises different scheduling patterns */
    for (int outer = 0; outer < iter_count; outer++) {
        /* Vary the patterns based on iteration */
        int pattern = outer % 4;
        
        switch (pattern) {
            case 0:
                /* Pattern 1: Dense arithmetic with barriers */
                total += dense_arithmetic(
                    v_seed1 + outer, 
                    v_seed2 - outer, 
                    v_seed3 ^ outer, 
                    total
                );
                break;
                
            case 1:
                /* Pattern 2: Branching with unpredictable control flow */
                total += branching_pattern(
                    10 + (outer % 7),  /* Varying iteration count */
                    total ^ v_seed1
                );
                break;
                
            case 2:
                /* Pattern 3: Vector operations */
                total += vector_operations(
                    v_seed1, 
                    v_seed2 + outer, 
                    v_seed3, 
                    total
                );
                break;
                
            case 3:
                /* Pattern 4: Inline assembly with constraints */
                total += inline_asm_constraints(
                    v_seed1 ^ outer, 
                    v_seed2, 
                    v_seed3 + total
                );
                break;
        }
        
        /* Occasionally call helper with barrier */
        if (outer % 7 == 0) {
            total = helper_with_barrier(total, outer) & 0x7FFFFFFF;
        }
        
        /* Update volatile to prevent optimization */
        v_counter = outer;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
