/* test_sched_context.c
 * A test program designed to trigger GCC's Haifa scheduler context
 * allocation and deallocation logic, specifically targeting the
 * free_sched_context function cleanup code.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function with data dependencies to create scheduling complexity */
static int helper_complex(int a, int b, int c) {
    int t1 = a * b + c;
    int t2 = (a ^ b) | c;
    int t3 = t1 * t2 - a;
    int t4 = (t3 << 3) | (t2 >> 2);
    return t4 * t1 + t2;
}

/* Pure function that acts as a scheduling barrier */
static int pure_helper(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    /* Create many independent operations to give scheduler work */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = e << 2;
    int t5 = t3 | t4;
    int t6 = t2 - t1;
    int t7 = t5 * t6;
    int t8 = t4 + t3;
    int t9 = t7 ^ t8;
    int t10 = t9 * a;
    int t11 = b + t10;
    int t12 = c ^ t11;
    int t13 = d * t12;
    int t14 = e | t13;
    int t15 = t14 - t9;
    int t16 = t15 * t8;
    int t17 = t16 >> 3;
    int t18 = t17 & 0xFF;
    int t19 = t18 + t7;
    int t20 = t19 * t6;
    
    /* More operations to ensure large basic block */
    int t21 = t20 / (t5 + 1);
    int t22 = t21 ^ t4;
    int t23 = t22 * t3;
    int t24 = t23 - t2;
    int t25 = t24 | t1;
    int t26 = t25 * e;
    int t27 = t26 + d;
    int t28 = t27 ^ c;
    int t29 = t28 * b;
    int t30 = t29 - a;
    
    return t30;
}

/* Function using SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(d, c, b, a);
    __m128 vec3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Multiple vector operations to create scheduling opportunities */
    __m128 res1 = _mm_add_ps(vec1, vec2);
    __m128 res2 = _mm_mul_ps(res1, vec3);
    __m128 res3 = _mm_sub_ps(vec2, res2);
    __m128 res4 = _mm_add_ps(res3, _mm_set1_ps(5.0f));
    
    /* Horizontal add pattern that creates dependencies */
    res4 = _mm_add_ps(res4, _mm_shuffle_ps(res4, res4, _MM_SHUFFLE(2, 3, 0, 1)));
    res4 = _mm_add_ps(res4, _mm_shuffle_ps(res4, res4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, res4);
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation with inline asm */
    asm volatile (
        "addl %1, %0\n\t"
        "imull %2, %0"
        : "=r" (res1)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    /* Memory barrier that scheduler cannot move across */
    asm volatile ("" ::: "memory");
    
    /* Second computation */
    asm volatile (
        "xorl %1, %0\n\t"
        "orl %2, %0"
        : "=r" (res2)
        : "0" (res1), "r" (c)
        : "cc"
    );
    
    /* Another barrier */
    asm volatile ("" ::: "memory");
    
    /* Third computation with different constraints */
    asm volatile (
        "movl %1, %0\n\t"
        "shrl $3, %0\n\t"
        "andl $0x0F, %0"
        : "=r" (res3)
        : "r" (res2)
        : "cc"
    );
    
    return res3;
}

/* Function with loop containing volatile to create unpredictable control flow */
static int volatile_loop_ops(int base, int iterations) {
    volatile int counter = iterations;
    int result = base;
    
    for (int i = 0; i < counter; ++i) {
        /* Mix of operations inside loop */
        int temp = pure_helper(result, i);
        
        /* Inline asm with dependencies */
        int asm_res;
        asm volatile (
            "movl %1, %0\n\t"
            "leal (%0,%0,2), %0\n\t"  /* result * 3 */
            "addl $7, %0"
            : "=r" (asm_res)
            : "r" (temp)
            : "cc"
        );
        
        /* Memory operation that scheduler must respect */
        volatile int mem_var = asm_res;
        result ^= mem_var;
        
        /* Conditional that depends on volatile */
        if (mem_var & 1) {
            result += helper_complex(result, i, mem_var);
        } else {
            result -= asm_res;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile float fseed1 = (float)(seed1 % 100) / 10.0f;
    volatile float fseed2 = (float)(seed2 % 100) / 10.0f;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of scheduler creating contexts */
    for (int iter = 0; iter < 100; ++iter) {
        /* Call different patterns to exercise various scheduler behaviors */
        
        /* 1. Dense arithmetic to fill instruction queue */
        int res1 = dense_arithmetic(
            seed1 + iter, 
            seed2 - iter, 
            seed3 ^ iter,
            seed1 * iter,
            seed2 + iter * 2
        );
        
        /* 2. SSE operations for target-specific scheduling */
        float res2 = sse_operations(
            fseed1 + iter * 0.1f,
            fseed2 - iter * 0.1f,
            fseed1 * 1.5f,
            fseed2 * 0.5f
        );
        
        /* 3. Inline assembly with barriers */
        int res3 = asm_barrier_ops(
            seed1 ^ res1,
            seed2 + iter,
            seed3 - iter
        );
        
        /* 4. Complex helper with data dependencies */
        int res4 = helper_complex(res1, res3, iter);
        
        /* 5. Loop with volatile to create control flow complexity */
        int res5 = volatile_loop_ops(res4, 5 + (iter % 10));
        
        /* Mix results together */
        total += res1 ^ res3;
        total += (int)res2 * res4;
        total -= res5;
        
        /* Occasionally call pure_helper as scheduling barrier */
        if (iter % 7 == 0) {
            total ^= pure_helper(total, iter);
        }
        
        /* Memory operations to create load/store dependencies */
        volatile int mem_check = total;
        if (mem_check & 0x100) {
            total >>= 2;
        }
    }
    
    /* Final computation with mixed operations */
    int final_result = 0;
    for (int i = 0; i < 20; ++i) {
        /* Create a micro-pattern that scheduler might optimize */
        int a = total + i;
        int b = total - i;
        int c;
        
        asm volatile (
            "movl %1, %0\n\t"
            "imull %2, %0\n\t"
            "addl $0x1234, %0"
            : "=r" (c)
            : "r" (a), "r" (b)
            : "cc"
        );
        
        final_result += c;
        
        /* Memory clobber to potentially cause scheduler backtracking */
        asm volatile ("" ::: "memory");
        
        /* Additional computation after barrier */
        final_result ^= helper_complex(final_result, i, c);
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    return final_result & 0xFF;
}
