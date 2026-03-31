/* test_sched_context.c - Trigger free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Volatile seeds to prevent constant propagation */
static volatile int seed1, seed2, seed3;
static volatile int iter_count;

/* Helper with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a * c + b;
    t5 = b * d - a;
    t6 = t3 & t4;
    t7 = t5 | t6;
    t8 = t2 + t7;
    t9 = t1 * t8;
    t10 = t9 / (c + 1);
    
    /* More operations with dependencies */
    t11 = t10 << 2;
    t12 = t11 >> 1;
    t13 = t12 + t3;
    t14 = t13 * t4;
    t15 = t14 - t5;
    t16 = t15 & 0xFF;
    t17 = t16 | 0x80;
    t18 = t17 ^ t6;
    t19 = t18 + t7;
    t20 = t19 * t8;
    
    /* Memory operations to introduce dependencies */
    volatile int mem1 = t20;
    int mem2 = mem1 + 1;
    
    /* Final mix */
    return (t9 + t20 + mem2) & 0xFFFF;
}

/* Helper with inline assembly barriers - forces state restoration */
static int asm_barrier_ops(int a, int b, int c) {
    int result = a;
    
    /* Sequence with artificial barriers */
    result += b;
    
    /* Opaque barrier - scheduler may try to move across this */
    asm volatile ("" : : : "memory");
    
    result *= c;
    
    /* Another barrier with register clobber */
    asm volatile ("# dummy" : : : "%eax", "memory");
    
    /* Dependent operations separated by barriers */
    for (int i = 0; i < (seed2 & 3); ++i) {
        result ^= (b << i);
        asm volatile ("" : "+r"(result) : : "memory");
    }
    
    return result;
}

/* Helper with vector operations - triggers target-specific scheduling */
static int vector_ops(int a, int b, int c, int d) {
    /* Create vector data */
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Mixed vector operations */
    __m128 sum = _mm_add_ps(v1, v2);
    __m128 mul = _mm_mul_ps(sum, v3);
    __m128 sub = _mm_sub_ps(mul, v1);
    
    /* Horizontal add pattern - creates dependencies */
    __m128 shuf = _mm_shuffle_ps(sub, sub, _MM_SHUFFLE(2, 3, 0, 1));
    __m128 adds = _mm_add_ps(sub, shuf);
    shuf = _mm_movehl_ps(shuf, adds);
    adds = _mm_add_ss(adds, shuf);
    
    /* Convert to integer */
    float f;
    _mm_store_ss(&f, adds);
    
    /* Integer operations mixed with vector results */
    int vi = (int)f;
    vi += a * b;
    vi ^= c | d;
    
    return vi & 0xFF;
}

/* Helper with branching and speculative scheduling */
static int branching_ops(int a, int b) {
    int result = 0;
    
    /* Loop with volatile condition - scheduler may save context at branches */
    for (int i = 0; i < (seed3 & 7); ++i) {
        /* Unpredictable branch */
        if ((a + i) & 1) {
            /* Complex sequence in taken branch */
            int t = a * i + b;
            t ^= t >> 3;
            t += (b << 2);
            result += t;
            
            /* Memory operation in branch */
            volatile int branch_mem = t;
            result ^= branch_mem;
        } else {
            /* Different sequence in else branch */
            int t = b - i * a;
            t = (t << 1) | (t >> 31);
            result -= t;
            
            /* Inline asm in else path */
            asm volatile ("# branch_else" : "+r"(t) : : "memory");
            result ^= t;
        }
        
        /* Small function call acts as scheduling barrier */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Helper with mixed operations and loops */
static int mixed_scheduling(int a, int b, int c) {
    int acc = 0;
    
    /* Outer loop with varying trip count */
    for (int i = 0; i < (iter_count & 3) + 2; ++i) {
        int local = a + i;
        
        /* Inner sequence with dependencies */
        for (int j = 0; j < 4; ++j) {
            local = (local * 1664525 + 1013904223) & 0xFFFF;
            
            /* Mix in some inline asm */
            asm volatile ("# inner_loop" : "+r"(local) : : "cc");
            
            /* Memory operation */
            volatile int mem = local;
            local ^= mem;
        }
        
        /* Conditional with both paths having complex ops */
        if (local & 1) {
            /* Vector ops in one path */
            __m128i v = _mm_set1_epi32(local);
            __m128i v2 = _mm_slli_epi32(v, 2);
            int arr[4];
            _mm_storeu_si128((__m128i*)arr, v2);
            acc += arr[0] + arr[1];
        } else {
            /* Integer ops in other path */
            acc += (local * b) >> (c & 3);
        }
        
        /* Dense arithmetic between iterations */
        acc += dense_arithmetic(local, b, c, acc);
    }
    
    return acc;
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant folding */
    seed1 = (argv[0] ? argv[0][0] : 'A');
    seed2 = (argc > 1 && argv[1]) ? argv[1][0] : 'B';
    seed3 = (argc > 2 && argv[2]) ? argv[2][0] : 'C';
    iter_count = (argc > 3) ? atoi(argv[3]) : 100;
    
    if (iter_count <= 0) iter_count = 100;
    
    int total = 0;
    
    /* Main loop - each iteration may create scheduling contexts */
    for (int iter = 0; iter < iter_count; ++iter) {
        /* Vary inputs each iteration */
        int a = seed1 + iter;
        int b = seed2 * iter;
        int c = seed3 ^ iter;
        int d = (seed1 + seed2 + seed3) & iter;
        
        /* Call different helpers to exercise various scheduling scenarios */
        total += dense_arithmetic(a, b, c, d);
        total += asm_barrier_ops(total, b, c);
        total += vector_ops(total & 0xFF, b, c, d);
        total += branching_ops(total, a);
        total += mixed_scheduling(total, b, c);
        
        /* Occasionally reset with volatile operation */
        if ((iter & 15) == 0) {
            volatile int reset = total;
            total = reset & 0xFFFF;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 1;
}
