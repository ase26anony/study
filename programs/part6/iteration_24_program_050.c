/* test_sched_context.c - Test program to trigger free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Helper function with pure computation to encourage scheduling */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
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
    int t14 = t13 ^ t1;
    int t15 = t14 & t2;
    int t16 = t15 | t3;
    int t17 = t16 << 3;
    int t18 = t17 >> 1;
    int t19 = t18 + t4;
    int t20 = t19 - t5;
    return t20;
}

/* Function with SSE intrinsics to trigger target-specific scheduling */
static float sse_computation(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Multiple dependent SSE operations */
    __m128 res1 = _mm_add_ps(vec1, vec2);
    __m128 res2 = _mm_mul_ps(res1, vec3);
    __m128 res3 = _mm_sub_ps(res2, vec1);
    __m128 res4 = _mm_add_ps(res3, _mm_set1_ps(1.5f));
    
    /* Extract result */
    float result[4];
    _mm_storeu_ps(result, res4);
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_computation(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation with memory clobber */
    asm volatile (
        "addl %1, %0\n\t"
        : "=r" (res1)
        : "r" (a), "0" (b)
        : "memory"
    );
    
    /* Opaque barrier that scheduler can't see through */
    asm volatile ("" : : : "memory");
    
    /* Second computation */
    asm volatile (
        "imull %1, %0\n\t"
        : "=r" (res2)
        : "r" (c), "0" (res1)
        : "memory"
    );
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Third computation */
    asm volatile (
        "xorl %1, %0\n\t"
        : "=r" (res3)
        : "r" (a), "0" (res2)
        : "memory"
    );
    
    return res3;
}

/* Function with loop containing volatile to create scheduling contexts */
static int volatile_loop_computation(volatile int iter, int a, int b) {
    int sum = 0;
    
    /* Loop with volatile iteration count - scheduler may create
       different contexts for different trip counts */
    for (int i = 0; i < iter; ++i) {
        /* Mix of operations with volatile accesses */
        volatile int vi = i;
        int tmp1 = a + vi;
        int tmp2 = b * vi;
        
        /* Function call acts as scheduling barrier */
        tmp1 = helper_pure(tmp1, tmp2);
        
        /* Memory operation */
        volatile int vtmp = tmp1;
        tmp2 = vtmp ^ b;
        
        sum += tmp2;
    }
    
    return sum;
}

/* Function with branching to create control flow for speculative scheduling */
static int branching_computation(int a, int b, int c, int seed) {
    int result = 0;
    
    /* Unpredictable branch based on computation */
    if ((a ^ seed) > (b & seed)) {
        /* First path with inline asm */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result)
            : "r" (a), "r" (b)
            : "%eax", "memory"
        );
        
        /* Dense computation in taken branch */
        result = dense_arithmetic(result, c, seed, a);
    } else {
        /* Alternative path with different operations */
        asm volatile (
            "movl %1, %%eax\n\t"
            "subl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (result)
            : "r" (b), "r" (a)
            : "%eax", "memory"
        );
        
        /* SSE computation in not-taken branch */
        float fresult = sse_computation(result, c, seed, a);
        result += (int)fresult;
    }
    
    /* Common tail with more operations */
    for (int i = 0; i < 3; ++i) {
        result = helper_pure(result, i + seed);
    }
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int loop_iter = argc > 4 ? atoi(argv[4]) : 100;
    
    int total = 0;
    
    /* Main loop to increase chance of scheduler creating/freeing contexts */
    for (volatile int outer = 0; outer < loop_iter; ++outer) {
        int vouter = outer;
        
        /* Call different computation patterns in sequence */
        total += dense_arithmetic(seed1 + vouter, seed2, seed3, vouter);
        
        total += sse_computation(seed1 * 0.1f, seed2 * 0.2f, 
                                seed3 * 0.3f, vouter * 0.4f);
        
        total += asm_barrier_computation(seed1, seed2 + vouter, seed3);
        
        /* Vary the iteration count for volatile loop */
        volatile int inner_iter = (vouter % 10) + 5;
        total += volatile_loop_computation(inner_iter, seed1, seed2 + vouter);
        
        /* Branching computation with seed from loop */
        total += branching_computation(seed1, seed2, seed3, vouter);
        
        /* Occasionally add more complex pattern */
        if (vouter % 7 == 0) {
            /* Nested loops with mixed operations */
            for (int j = 0; j < 3; ++j) {
                int tmp = 0;
                for (int k = 0; k < 5; ++k) {
                    tmp += helper_pure(seed1 + j, seed2 + k);
                    tmp ^= asm_barrier_computation(tmp, seed3, j * k);
                }
                total += tmp;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
