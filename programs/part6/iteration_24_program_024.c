/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling contexts */

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    volatile int v = a; /* Prevent optimization */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations */
    t1 = v + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = b << 2;
    t5 = c >> 1;
    t6 = t3 + t4;
    t7 = t5 * e;
    t8 = t6 - t7;
    t9 = t8 & 0xFF;
    t10 = t9 | 0x80;
    
    /* More operations to increase instruction density */
    t11 = t10 * 3;
    t12 = t11 / 2;
    t13 = t12 % 17;
    t14 = t13 << 3;
    t15 = t14 >> 1;
    t16 = t15 + 42;
    t17 = t16 * t1;
    t18 = t17 - t2;
    t19 = t18 ^ t3;
    t20 = t19 & 0xFFFF;
    
    /* Memory operations to introduce dependencies */
    volatile int mem1 = t20;
    int mem2 = mem1;
    
    t1 = mem2 + t4;
    t2 = t5 * mem2;
    t3 = t1 ^ t2;
    
    return t3 + t20;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_test(int a, int b, int c) {
    int result = 0;
    volatile int barrier = 0;
    
    /* Initial computation */
    int x = a * b + c;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : "+r" (x) : : "memory");
    
    /* Dependent computation */
    int y = x / 2;
    
    /* Another barrier */
    asm volatile ("# barrier" : : : "memory");
    
    /* More computations with volatile reads */
    barrier = y;
    int z = barrier * 3;
    
    /* Complex sequence with multiple barriers */
    for (int i = 0; i < (barrier & 0x3); ++i) {
        int tmp = z + i;
        asm volatile ("" : "+r" (tmp) : : "memory");
        result += tmp;
    }
    
    return result;
}

/* Function with vector operations to trigger target-specific scheduling */
static int vector_ops_test(int a, int b, int c, int d) {
    /* Use SSE intrinsics to engage target-specific scheduling */
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set1_ps(2.0f);
    
    /* Multiple vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    
    /* Mix with scalar operations */
    float f[4];
    _mm_storeu_ps(f, r3);
    
    int sum = 0;
    for (int i = 0; i < 4; ++i) {
        sum += (int)f[i];
    }
    
    /* More vector operations in a loop */
    __m128i vi1 = _mm_set_epi32(a, b, c, d);
    __m128i vi2 = _mm_set_epi32(1, 2, 3, 4);
    
    for (int i = 0; i < 8; ++i) {
        vi1 = _mm_add_epi32(vi1, vi2);
        /* Inline assembly to prevent optimization */
        asm volatile ("" : "+x" (vi1));
    }
    
    int vi_arr[4];
    _mm_storeu_si128((__m128i*)vi_arr, vi1);
    
    return sum + vi_arr[0] + vi_arr[3];
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_test(volatile int limit) {
    int total = 0;
    
    /* Loop with volatile limit to prevent unrolling */
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable condition */
        if (i & 1) {
            /* Branch 1: Complex computation */
            int x = i * 3;
            int y = x << 2;
            int z = y ^ 0x55;
            total += z;
            
            /* Inline assembly with dependencies */
            asm volatile ("addl %1, %0" : "+r" (total) : "r" (x));
        } else {
            /* Branch 2: Different computation */
            int x = i + 5;
            int y = x * x;
            int z = y % 17;
            total -= z;
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        
        /* Additional computation common to both paths */
        total = (total * 1103515245 + 12345) & 0x7fffffff;
    }
    
    return total;
}

/* Function mixing all patterns in nested loops */
static int mixed_patterns_test(int base, volatile int iter) {
    int result = 0;
    
    /* Outer loop with volatile iteration count */
    for (int outer = 0; outer < iter; ++outer) {
        int a = base + outer;
        int b = a * 2;
        int c = b + 1;
        int d = c ^ 0xFF;
        
        /* Call different pattern functions */
        result += dense_arithmetic(a, b, c, d, outer);
        
        if (outer & 1) {
            result += asm_barrier_test(a, b, c);
        } else {
            result += vector_ops_test(a, b, c, d);
        }
        
        /* Inner loop with assembly operations */
        for (int inner = 0; inner < (outer & 0x7); ++inner) {
            int tmp;
            /* Inline assembly with multiple outputs */
            asm volatile (
                "imull %2, %1\n\t"
                "addl %1, %0\n\t"
                "xorl %3, %0"
                : "+r" (result), "=&r" (tmp)
                : "r" (inner), "r" (a)
                : "cc"
            );
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Use argv to create volatile seeds preventing constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int loop_limit = argc > 4 ? atoi(argv[4]) : 100;
    
    int total = 0;
    
    /* Main loop to increase chance of scheduler context creation */
    for (int main_iter = 0; main_iter < loop_limit; ++main_iter) {
        /* Vary the patterns based on iteration */
        volatile int pattern_selector = (main_iter + seed1) & 0x3;
        
        switch (pattern_selector) {
            case 0:
                total += dense_arithmetic(
                    seed1 + main_iter,
                    seed2 - main_iter,
                    seed3 * main_iter,
                    (seed1 ^ seed2) + main_iter,
                    (seed2 ^ seed3) - main_iter
                );
                break;
                
            case 1:
                total += asm_barrier_test(
                    seed1 + main_iter,
                    seed2 * 2,
                    seed3 >> 1
                );
                break;
                
            case 2:
                total += vector_ops_test(
                    seed1,
                    seed2 + main_iter,
                    seed3,
                    main_iter
                );
                break;
                
            case 3:
                total += branching_test((main_iter & 0x1F) + 10);
                break;
        }
        
        /* Periodically call the mixed patterns function */
        if ((main_iter % 17) == 0) {
            total += mixed_patterns_test(seed1 + seed2, (main_iter & 0x7) + 3);
        }
        
        /* Introduce unpredictable control flow */
        if (total & (1 << 16)) {
            /* Additional computation on certain iterations */
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %%eax\n\t"
                "addl %%eax, %0"
                : "+r" (total)
                : "r" (main_iter)
                : "%eax", "cc"
            );
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
