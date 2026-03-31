/* test_sched_context.c - Test program to trigger haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Helper function with pure computation - scheduler may try to move it */
static int pure_helper(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(volatile int a, volatile int b, volatile int c, volatile int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to give scheduler lots of work */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a - c;
    t5 = b + d;
    t6 = t4 * t5;
    t7 = t3 & t6;
    t8 = a << 2;
    t9 = b >> 1;
    t10 = t8 | t9;
    t11 = c + 0x1234;
    t12 = d - 0x5678;
    t13 = t11 * t12;
    t14 = t10 ^ t13;
    t15 = t7 + t14;
    t16 = a * c + b * d;
    t17 = t15 - t16;
    t18 = (a + c) * (b + d);
    t19 = t17 | t18;
    t20 = t19 & 0xFFFF;
    
    /* Memory operations to introduce dependencies */
    volatile int mem1 = t20;
    volatile int mem2 = mem1 + 1;
    
    return t20 + mem2;
}

/* Function with SSE/vector operations to trigger target-specific scheduling */
static float vector_operations(volatile float f1, volatile float f2, 
                               volatile float f3, volatile float f4) {
    __m128 v1 = _mm_set_ps(f1, f2, f3, f4);
    __m128 v2 = _mm_set_ps(f4, f3, f2, f1);
    __m128 v3 = _mm_set1_ps(2.0f);
    
    /* Chain of vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_mul_ps(r3, _mm_set1_ps(0.5f));
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, r4);
    
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers to force scheduler backtracking */
static int asm_barrier_test(volatile int x, volatile int y) {
    int r1, r2, r3, r4;
    
    /* Initial computation */
    r1 = x * y;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : "+r" (r1) : : "memory");
    
    /* More computations after barrier */
    r2 = r1 + (x ^ y);
    
    /* Another barrier with different constraints */
    asm volatile ("# Dummy asm\n\t" 
                  "addl %1, %0\n\t"
                  : "=r" (r3) 
                  : "r" (r2), "0" (x) 
                  : "cc", "memory");
    
    /* Complex dependency chain */
    r4 = r3;
    for (int i = 0; i < 3; i++) {
        asm volatile ("# Loop barrier %0\n\t" 
                      : "+r" (r4) : : "memory");
        r4 = pure_helper(r4, y + i);
    }
    
    return r4;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_test(volatile int a, volatile int b, volatile int cond) {
    int result = 0;
    
    /* Loop with volatile condition - scheduler can't predict trip count */
    for (int i = 0; i < (cond & 0x7); ++i) {
        /* Mix of operations inside loop */
        int t = a + i;
        t = t * b;
        
        /* Conditional with side effect */
        if (t & 1) {
            asm volatile ("# True branch\n\t" : : : "memory");
            result += t;
        } else {
            asm volatile ("# False branch\n\t" : : : "memory");
            result -= t;
        }
        
        /* Call to pure function inside loop */
        result += pure_helper(result, t);
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int seed1, seed2, seed3, seed4;
    volatile float fseed1, fseed2, fseed3, fseed4;
    int total = 0;
    
    /* Initialize from argv to prevent constant propagation */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 24680;
    seed4 = (argc > 4) ? atoi(argv[4]) : 13579;
    
    fseed1 = seed1 * 0.01f;
    fseed2 = seed2 * 0.02f;
    fseed3 = seed3 * 0.03f;
    fseed4 = seed4 * 0.04f;
    
    /* Execute multiple iterations to increase chance of context creation */
    for (int iter = 0; iter < 100; iter++) {
        volatile int cond = seed1 + iter;
        
        /* Call different test functions to exercise various scheduler paths */
        total += dense_arithmetic(seed1 + iter, seed2 - iter, 
                                  seed3 ^ iter, seed4 | iter);
        
        total += asm_barrier_test(seed2 + iter, seed3 - iter);
        
        total += branching_test(seed1 ^ iter, seed2 | iter, cond);
        
        /* Every few iterations, do vector operations */
        if (iter % 7 == 0) {
            float fresult = vector_operations(fseed1 + iter, fseed2 - iter,
                                            fseed3 * 0.5f, fseed4 * 0.25f);
            total += (int)fresult;
        }
        
        /* Occasionally add more complex patterns */
        if (iter % 13 == 0) {
            /* Nested loop with volatile limit */
            volatile int limit = (seed4 + iter) & 0x3;
            for (int j = 0; j < limit; j++) {
                total += pure_helper(total, seed1 + j);
                
                /* Inline assembly with dependencies */
                int tmp = total;
                asm volatile ("imull %1, %0\n\t"
                              "addl $0x1234, %0\n\t"
                              : "+r" (tmp) 
                              : "r" (seed2 + j) 
                              : "cc");
                total = tmp;
            }
        }
        
        /* Modify seeds to create varying patterns */
        seed1 = seed1 ^ (total & 0xFF);
        seed2 = seed2 + (iter * 37);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
