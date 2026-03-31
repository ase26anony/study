#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling complexity */

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t3 - e;
    int t5 = t4 * a;
    int t6 = t5 / (b + 1);
    int t7 = t6 << 2;
    int t8 = t7 & 0xFF;
    int t9 = t8 | c;
    int t10 = t9 * d;
    int t11 = t10 + e;
    int t12 = t11 - a;
    int t13 = t12 * b;
    int t14 = t13 / (c + 1);
    int t15 = t14 ^ d;
    int t16 = t15 << 1;
    int t17 = t16 & 0x7F;
    int t18 = t17 | e;
    int t19 = t18 * a;
    int t20 = t19 + b;
    
    /* Memory operations to introduce dependencies */
    volatile int mem1 = t20;
    int t21 = mem1 * c;
    volatile int mem2 = t21;
    int t22 = mem2 + d;
    
    return t22;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(int x, int y, int z) {
    int result;
    
    /* First computation */
    int tmp1 = x * y;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    int tmp2 = tmp1 + z;
    
    /* Another barrier */
    asm volatile ("# This is a comment barrier" : : : "memory");
    
    /* More computations with artificial dependencies */
    for (int i = 0; i < 3; i++) {
        asm volatile ("nop" : : : "memory");
        tmp2 = tmp2 * (x + i);
    }
    
    result = tmp2;
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function using SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    
    /* Mix of SSE operations */
    __m128 vec3 = _mm_add_ps(vec1, vec2);
    __m128 vec4 = _mm_mul_ps(vec1, vec2);
    __m128 vec5 = _mm_sub_ps(vec3, vec4);
    
    /* Horizontal add pattern */
    vec5 = _mm_add_ps(vec5, _mm_shuffle_ps(vec5, vec5, _MM_SHUFFLE(2, 3, 0, 1)));
    vec5 = _mm_add_ps(vec5, _mm_shuffle_ps(vec5, vec5, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, vec5);
    
    return result;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(int base, int limit) {
    int total = 0;
    volatile int mod = (limit % 7) + 1;  /* Prevent optimization */
    
    for (int i = 0; i < limit; i++) {
        /* Unpredictable condition */
        if ((i % mod) == 0) {
            /* Complex computation in taken branch */
            int t = base * i;
            t = t ^ (t >> 3);
            t = t * 0x9e3779b9;
            total += t;
            
            /* Inline asm to prevent reordering */
            asm volatile ("" : : : "memory");
        } else {
            /* Different computation in not-taken branch */
            int t = base + i;
            t = t * 0x85ebca6b;
            t = t ^ (t << 13);
            total -= t;
        }
        
        /* Occasionally add a memory barrier */
        if ((i & 0x3F) == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return total;
}

/* Function with mixed operations and loop-carried dependencies */
static int mixed_loop_operations(int seed, int iterations) {
    int a = seed;
    int b = seed * 2;
    int c = seed * 3;
    
    volatile int iter = iterations;  /* Prevent loop unrolling */
    
    for (int i = 0; i < iter; i++) {
        /* Independent operations that can be scheduled in parallel */
        int op1 = a + b;
        int op2 = c * i;
        int op3 = a ^ b;
        int op4 = c - i;
        
        /* Create artificial dependencies */
        a = op1 + op2;
        b = op3 * op4;
        c = (a ^ b) + i;
        
        /* Memory operation to force ordering */
        volatile int mem = c;
        a = a + mem;
        
        /* Inline assembly with register constraints */
        int temp;
        asm volatile ("addl %1, %0" : "=r"(temp) : "r"(a), "0"(b));
        b = temp;
        
        /* Another memory barrier every 8 iterations */
        if ((i & 7) == 0) {
            asm volatile ("# Loop barrier" : : : "memory");
        }
    }
    
    return a + b + c;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile float fseed = (float)(seed1 % 100) / 10.0f;
    
    int total = 0;
    
    /* Execute multiple iterations to increase chance of scheduling contexts */
    volatile int outer_iterations = 100;
    
    for (int outer = 0; outer < outer_iterations; outer++) {
        /* Pattern 1: Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1 + outer, seed2, seed3, outer, seed1);
        
        /* Pattern 2: Assembly barriers for state restoration */
        total += asm_barrier_ops(seed2 + outer, seed3, seed1);
        
        /* Pattern 3: SSE operations for target-specific scheduling */
        float fresult = sse_operations(fseed + outer, fseed * 2, 
                                      fseed * 3, fseed * 4);
        total += (int)fresult;
        
        /* Pattern 4: Unpredictable branching */
        total += branching_pattern(seed3 + outer, 50 + (outer % 30));
        
        /* Pattern 5: Mixed operations in loops */
        total += mixed_loop_operations(seed1 + seed2 + outer, 20 + (outer % 10));
        
        /* Occasionally add a global memory barrier */
        if ((outer & 0xF) == 0) {
            asm volatile ("# Global barrier" : : : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
