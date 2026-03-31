#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling complexity */

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, volatile int iter) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Create many independent operations to give scheduler choices */
        t1 = a + b;
        t2 = c * d;
        t3 = t1 ^ t2;
        t4 = a * c;
        t5 = b + d;
        t6 = t4 - t5;
        t7 = t3 | t6;
        t8 = a << 2;
        t9 = b >> 1;
        t10 = t8 & t9;
        t11 = c + d;
        t12 = a * b;
        t13 = t11 * t12;
        t14 = t10 ^ t13;
        t15 = t7 + t14;
        t16 = d * 3;
        t17 = c - a;
        t18 = t16 / (t17 ? t17 : 1);
        t19 = t15 | t18;
        t20 = t19 * 2;
        
        /* Memory operations to introduce load/store dependencies */
        volatile int mem1 = t20;
        volatile int mem2 = mem1 + i;
        t1 = mem2 * 7;
        
        /* More arithmetic chains */
        t2 = t1 + a;
        t3 = t2 * b;
        t4 = t3 - c;
        t5 = t4 ^ d;
        
        result += t5;
        
        /* Modify inputs slightly for next iteration */
        a += 1;
        b ^= i;
        c = c * 2 - 1;
        d = d + (i & 3);
    }
    
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(int a, int b, int c, volatile int iter) {
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        int t1, t2, t3;
        
        /* Initial computation */
        t1 = a * b + c;
        
        /* Assembly barrier that looks like it might be movable */
        asm volatile ("# BEGIN BARRIER" ::: "memory");
        
        /* Dependent computation */
        t2 = t1 * 2 - b;
        
        /* Another barrier */
        asm volatile ("# MIDDLE BARRIER" ::: "memory");
        
        /* More computation */
        t3 = t2 ^ a;
        
        /* Final barrier */
        asm volatile ("# END BARRIER" ::: "memory");
        
        result += t3;
        
        /* Volatile read to prevent optimization */
        volatile int v = iter;
        a += v & 1;
        b ^= i;
        c = c * 3 - 2;
    }
    
    return result;
}

/* Function with vector operations to trigger target-specific scheduling */
static int vector_operations(int a, int b, int c, int d, volatile int iter) {
    /* Use volatile to prevent constant propagation */
    volatile float va = a * 0.5f;
    volatile float vb = b * 1.5f;
    volatile float vc = c * 2.5f;
    volatile float vd = d * 3.5f;
    
    __m128 vec1 = _mm_set_ps(va, vb, vc, vd);
    __m128 vec2 = _mm_set1_ps(2.0f);
    __m128 result_vec = _mm_setzero_ps();
    
    for (int i = 0; i < iter; ++i) {
        /* Mix of vector operations */
        __m128 temp1 = _mm_add_ps(vec1, vec2);
        __m128 temp2 = _mm_mul_ps(temp1, vec1);
        
        /* Scalar operation in between to create scheduling complexity */
        volatile int scalar = i * a;
        
        __m128 temp3 = _mm_sub_ps(temp2, _mm_set1_ps(scalar * 0.01f));
        result_vec = _mm_add_ps(result_vec, temp3);
        
        /* Modify vector for next iteration */
        vec1 = _mm_add_ps(vec1, _mm_set1_ps(0.1f));
        
        /* Branch with unpredictable outcome */
        if (scalar & 0x7) {
            vec2 = _mm_mul_ps(vec2, _mm_set1_ps(1.01f));
        }
    }
    
    /* Extract result */
    float result_array[4];
    _mm_store_ps(result_array, result_vec);
    return (int)(result_array[0] + result_array[1] + result_array[2] + result_array[3]);
}

/* Function with branching and speculative scheduling opportunities */
static int branching_ops(int a, int b, volatile int iter) {
    int result = 0;
    volatile int threshold = iter / 2;
    
    for (int i = 0; i < iter; ++i) {
        int x = a + i;
        int y = b - i;
        
        /* Unpredictable branch */
        if (x > threshold) {
            /* Complex computation in hot path */
            int t1 = x * y;
            int t2 = t1 << 3;
            int t3 = t2 ^ 0xABCD;
            int t4 = t3 * 7;
            result += t4;
            
            /* Inline assembly with dependencies */
            asm volatile ("mov %0, %1\n\t"
                         "add %0, %2\n\t"
                         : "=r"(t1) : "r"(t3), "r"(t4));
            result += t1;
        } else {
            /* Different computation in cold path */
            int t1 = x + y;
            int t2 = t1 * 3;
            int t3 = t2 | 0xFF;
            result += t3;
            
            /* Memory operation */
            volatile int mem = t3;
            result += mem * 2;
        }
        
        /* Cross-iteration dependencies */
        a = (a * 3 + 1) & 0xFFF;
        b = (b * 5 - 2) & 0xFFF;
    }
    
    return result;
}

/* Function with mixed operations and function calls */
static int mixed_operations(int a, int b, int c, int d, volatile int iter) {
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Start with some arithmetic */
        int t1 = a * b + c;
        
        /* Memory store/load to create dependencies */
        volatile int storage;
        storage = t1;
        int t2 = storage + d;
        
        /* More arithmetic with different ops */
        int t3 = (t2 << 2) | (t1 >> 1);
        int t4 = t3 ^ 0x1234;
        
        /* Conditional operation */
        int t5 = (i & 1) ? t4 * 3 : t4 + 7;
        
        /* Complex expression with multiple dependencies */
        int t6 = (t5 * a) + (t4 * b) - (t3 * c) / (d ? d : 1);
        
        result += t6;
        
        /* Update variables for next iteration */
        a = (a + b) ^ 0x5555;
        b = (b - c) * 2;
        c = (c + d) | 0xAAAA;
        d = (d * 3) & 0xFFFF;
        
        /* Volatile read to prevent optimization across iterations */
        volatile int v = iter;
        if (v & 0x1) {
            result += 1;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 98765;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the iteration counts to create different scheduling scenarios */
        volatile int iter1 = (seed1 + outer) % 50 + 10;
        volatile int iter2 = (seed2 + outer) % 40 + 15;
        volatile int iter3 = (seed3 + outer) % 30 + 20;
        volatile int iter4 = (seed4 + outer) % 20 + 25;
        
        /* Call different types of functions to exercise various scheduler paths */
        total += dense_arithmetic(seed1 + outer, seed2, seed3, seed4, iter1);
        total += asm_barrier_ops(seed2 + outer, seed3, seed4, iter2);
        total += vector_operations(seed3 + outer, seed4, seed1, seed2, iter3);
        total += branching_ops(seed4 + outer, seed1, iter4);
        total += mixed_operations(seed1, seed2 + outer, seed3, seed4, iter1 % 20 + 5);
        
        /* Modify seeds to create varying patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed3 ^= outer * 0x9E3779B9;
        seed4 += outer * 0x6A09E667;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
