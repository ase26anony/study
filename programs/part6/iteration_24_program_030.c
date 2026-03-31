/* test_sched_context.c - Trigger Haifa scheduler context allocation/freeing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
volatile int seed1, seed2, seed3;
volatile int iter_count = 100;

/* Helper with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Independent arithmetic operations to create large ready list */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a * c + b;
    t5 = b * d - a;
    t6 = t1 & t2;
    t7 = t3 | t4;
    t8 = t5 ^ t6;
    t9 = t7 * t8;
    t10 = t9 + t1;
    
    /* More operations with dependencies */
    t11 = t10 >> 2;
    t12 = t11 * 3;
    t13 = t12 + t3;
    t14 = t13 & 0xFF;
    t15 = t14 * t5;
    t16 = t15 - t8;
    t17 = t16 | t9;
    t18 = t17 ^ t10;
    t19 = t18 + t12;
    t20 = t19 * 2;
    
    /* Memory operations to introduce load/store dependencies */
    volatile int mem1 = t20;
    int mem2 = mem1 + t15;
    volatile int mem3 = mem2;
    
    return t20 + mem3;
}

/* Helper with inline assembly barriers - may cause state restoration */
static int asm_barrier_ops(int a, int b, int c) {
    int result = a;
    
    /* Sequence with artificial barriers */
    result += b;
    
    /* Opaque assembly barrier - scheduler cannot see through this */
    asm volatile ("" : : : "memory");
    
    result *= c;
    
    /* Another barrier between dependent operations */
    asm volatile ("# barrier" : : : "memory");
    
    /* More arithmetic with volatile to prevent optimization */
    volatile int v = result;
    result = v + (b * 2);
    
    /* Complex inline assembly with dependencies */
    int out1, out2;
    asm volatile ("addl %2, %0\n\t"
                  "imull %3, %1"
                  : "=r"(out1), "=r"(out2)
                  : "r"(result), "r"(c), "0"(a), "1"(b)
                  : "cc");
    
    asm volatile ("" : : : "memory");
    
    return out1 + out2;
}

/* Helper with vector intrinsics - triggers target-specific scheduling */
static int vector_intrinsics_mix(int a, int b, int c, int d) {
    /* Create vector data */
    __m128i vec1 = _mm_set_epi32(a, b, c, d);
    __m128i vec2 = _mm_set_epi32(d, c, b, a);
    __m128i vec3 = _mm_set1_epi32(seed1);
    
    /* Mixed vector operations */
    __m128i res1 = _mm_add_epi32(vec1, vec2);
    __m128i res2 = _mm_mullo_epi32(res1, vec3);
    __m128i res3 = _mm_sub_epi32(vec2, res2);
    
    /* Scalar operations mixed with vector */
    int scalar = a + b + c + d;
    
    /* More vector ops */
    __m128i res4 = _mm_slli_epi32(res3, 2);
    __m128i res5 = _mm_xor_si128(res4, res1);
    
    /* Extract results to scalar */
    int results[4];
    _mm_storeu_si128((__m128i*)results, res5);
    
    /* Branch with unpredictable outcome */
    if (seed2 & 1) {
        /* Different vector path */
        __m128 resf1 = _mm_set_ps(a, b, c, d);
        __m128 resf2 = _mm_set_ps(d, c, b, a);
        __m128 resf3 = _mm_add_ps(resf1, resf2);
        float fresults[4];
        _mm_storeu_ps(fresults, resf3);
        scalar += (int)fresults[0];
    }
    
    return results[0] + results[1] + results[2] + results[3] + scalar;
}

/* Helper with branching and loops - creates multiple scheduling contexts */
static int branching_pattern(int base, int count) {
    int total = 0;
    volatile int mod = seed3;
    
    for (int i = 0; i < count; i++) {
        /* Unpredictable branch */
        if ((i % mod) == 0) {
            /* Dense block for scheduler */
            int a = base + i;
            int b = i * 2;
            int c = a ^ b;
            int d = c * 3;
            
            /* Inline asm with resource constraints */
            int r1, r2;
            asm volatile ("movl %2, %0\n\t"
                          "addl %3, %0\n\t"
                          "movl %4, %1\n\t"
                          "imull %0, %1"
                          : "=r"(r1), "=r"(r2)
                          : "r"(a), "r"(b), "r"(c)
                          : "cc");
            
            total += r1 + r2 + d;
        } else {
            /* Alternative path */
            total += (base - i) * 7;
        }
        
        /* Small pure function call acts as scheduling barrier */
        total += (total & 1) ? -1 : 1;
    }
    
    return total;
}

/* Complex loop with mixed operations - main trigger */
static int complex_scheduling_region(int a, int b, int c, int d) {
    int total = 0;
    
    /* Loop with volatile iteration count prevents unrolling */
    for (int i = 0; i < iter_count; i++) {
        /* Mix different patterns */
        int r1 = dense_arithmetic(a + i, b, c, d);
        
        /* Branch based on volatile */
        if (seed1 & (1 << (i % 8))) {
            r1 = asm_barrier_ops(r1, b, c);
        }
        
        int r2 = vector_intrinsics_mix(r1, b + i, c, d);
        
        /* Nested loop with small trip count */
        for (int j = 0; j < 4; j++) {
            r2 += branching_pattern(r2, 3);
        }
        
        total += r2;
        
        /* Modify arguments to create varying patterns */
        a = (a * 3 + 1) & 0xFF;
        b = (b + i) & 0xFF;
        c = (c ^ total) & 0xFF;
        d = (d * 5 - 1) & 0xFF;
    }
    
    return total;
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant propagation */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    /* Vary iteration count */
    iter_count = 50 + (seed1 % 50);
    
    int result = 0;
    
    /* Multiple passes to increase chance of scheduling contexts */
    for (int pass = 0; pass < 3; pass++) {
        /* Different initial values each pass */
        int a = seed1 + pass * 100;
        int b = seed2 - pass * 50;
        int c = seed3 ^ (pass * 0x99);
        int d = (seed1 + seed2 + seed3) & 0xFF;
        
        /* Call the complex region */
        result += complex_scheduling_region(a, b, c, d);
        
        /* Modify seeds between passes */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
