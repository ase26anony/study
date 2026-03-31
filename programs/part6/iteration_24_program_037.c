/* test_sched_context.c - Trigger free_sched_context coverage in haifa-sched.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Helper function with pure computation - creates scheduling barriers */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (b | c);
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, volatile int iter) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Create many independent operations to fill scheduler structures */
        t1 = a + b;
        t2 = c * d;
        t3 = t1 ^ t2;
        t4 = a - b;
        t5 = c + d;
        t6 = t4 | t5;
        t7 = t3 & t6;
        t8 = b << 2;
        t9 = c >> 1;
        t10 = t8 ^ t9;
        t11 = a * 3;
        t12 = d + 7;
        t13 = t11 % 13;
        t14 = t12 & 0xFF;
        t15 = t13 | t14;
        t16 = t10 + t15;
        t17 = t7 - t16;
        t18 = t17 * 2;
        t19 = t18 ^ 0xAAAA;
        t20 = t19 + i;
        
        /* Mix in memory operations */
        volatile int mem_var = t20;
        int mem_read = mem_var;
        
        /* More arithmetic chains */
        t1 = mem_read + a;
        t2 = t1 * b;
        t3 = t2 - c;
        t4 = t3 ^ d;
        t5 = t4 << (i & 3);
        t6 = t5 >> 1;
        t7 = t6 + mem_read;
        t8 = t7 * 3;
        t9 = t8 % 17;
        t10 = t9 | 0x55;
        
        result += t10;
        
        /* Modify inputs to prevent complete optimization */
        a = (a + 1) & 0xFF;
        b = (b ^ i) & 0xFF;
    }
    
    return result;
}

/* Function with inline assembly barriers - forces state restoration */
static int asm_barrier_sequence(int a, int b, volatile int flag) {
    int result = 0;
    
    for (int i = 0; i < 10; ++i) {
        int tmp1, tmp2, tmp3;
        
        /* Initial computation */
        tmp1 = a * i + b;
        
        /* Assembly barrier that looks schedulable but has hidden dependencies */
        asm volatile (
            "movl %1, %%eax\n\t"
            "addl %2, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "=r" (tmp2)
            : "r" (tmp1), "r" (i)
            : "%eax", "memory"  /* Memory clobber creates scheduling barrier */
        );
        
        /* Dependent computation */
        tmp3 = tmp2 ^ (a + b);
        
        /* Another barrier with register constraints */
        asm volatile (
            "imull %1, %0\n\t"
            "addl $0x1234, %0\n\t"
            : "+r" (tmp3)
            : "r" (flag)
            : "cc", "memory"
        );
        
        /* Control flow that might cause speculative scheduling */
        if (flag & (1 << (i & 3))) {
            /* Complex computation in taken branch */
            result += pure_helper(tmp3, a, b);
        } else {
            /* Different computation in not-taken branch */
            result -= tmp3 * 2;
        }
        
        /* Modify volatile flag to prevent prediction */
        asm volatile ("" : : "r"(flag) : "memory");
    }
    
    return result;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static int vector_intrinsics_mix(float a, float b, float c, float d, volatile int iter) {
    __m128 vec1, vec2, vec3, vec4;
    float result_arr[4] __attribute__((aligned(16)));
    int result = 0;
    
    /* Initialize vectors with volatile-dependent values */
    vec1 = _mm_set_ps(a, b, c, d);
    vec2 = _mm_set_ps(b, c, d, a);
    
    for (int i = 0; i < iter; ++i) {
        /* Mix different vector operations */
        vec3 = _mm_add_ps(vec1, vec2);
        vec4 = _mm_mul_ps(vec3, vec1);
        
        /* Horizontal operations that create dependencies */
        vec1 = _mm_add_ps(vec4, _mm_set1_ps(i * 0.1f));
        vec2 = _mm_sub_ps(vec1, _mm_set1_ps(1.0f));
        
        /* Store and reload - creates memory dependencies */
        _mm_store_ps(result_arr, vec1);
        
        /* Scalar operations mixed with vector */
        float temp = result_arr[i & 3];
        
        /* Integer SIMD using SSE2 */
        __m128i ivec1 = _mm_set1_epi32((int)(temp * 1000));
        __m128i ivec2 = _mm_set1_epi32(i);
        __m128i ivec3 = _mm_add_epi32(ivec1, ivec2);
        
        /* Extract results */
        int results[4] __attribute__((aligned(16)));
        _mm_store_si128((__m128i*)results, ivec3);
        
        result += results[0] + results[1] + results[2] + results[3];
        
        /* Modify vectors to prevent optimization */
        vec1 = _mm_add_ps(vec1, _mm_set1_ps(0.5f));
    }
    
    return result;
}

/* Function with unpredictable control flow - causes context saves at branches */
static int unpredictable_branches(volatile int seed, int a, int b) {
    int result = 0;
    int state = seed;
    
    for (int i = 0; i < 50; ++i) {
        /* Unpredictable condition based on volatile and computation */
        int cond = (state * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Complex computations in both branches */
        if (cond & 0x100) {
            /* Branch 1: Vector-like computation using integers */
            int t1 = a * i + b;
            int t2 = t1 ^ state;
            int t3 = (t2 << 3) | (t2 >> 29);
            int t4 = pure_helper(t3, a, b);
            
            /* Inline assembly that scheduler might try to move */
            asm volatile (
                "movl %1, %%ecx\n\t"
                "leal (%%ecx,%%ecx,2), %0\n\t"
                : "=r" (t4)
                : "r" (t4)
                : "%ecx"
            );
            
            result += t4;
        } else {
            /* Branch 2: Different computation pattern */
            int t1 = b * i - a;
            int t2 = t1 & state;
            int t3 = (t2 * 3) % 17;
            
            /* Memory operation that creates scheduling constraints */
            volatile int mem_barrier = t3;
            int t4 = mem_barrier + (i << 2);
            
            result -= t4;
        }
        
        /* Update state for next iteration */
        state = (state * 1664525 + 1013904223) & 0x7FFFFFFF;
        
        /* Insert scheduling barrier periodically */
        if (i % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    return result;
}

/* Main driver that combines all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int iter_count = argc > 4 ? atoi(argv[4]) : 100;
    
    int total_result = 0;
    
    /* Loop to increase chance of scheduler creating/freeing contexts */
    for (int outer = 0; outer < 3; ++outer) {
        /* Mix different scheduling patterns in each iteration */
        
        /* Pattern 1: Dense arithmetic to fill instruction queues */
        total_result += dense_arithmetic(
            seed1 + outer, 
            seed2, 
            seed3, 
            outer * 7, 
            iter_count % 50 + 10
        );
        
        /* Pattern 2: Assembly barriers for state restoration */
        total_result += asm_barrier_sequence(
            seed2 + outer, 
            seed3, 
            seed1 ^ outer
        );
        
        /* Pattern 3: Vector intrinsics for target-specific scheduling */
        total_result += vector_intrinsics_mix(
            (seed1 % 100) * 0.1f,
            (seed2 % 100) * 0.2f,
            (seed3 % 100) * 0.3f,
            (outer % 100) * 0.4f,
            iter_count % 20 + 5
        );
        
        /* Pattern 4: Unpredictable branches for speculative scheduling */
        total_result += unpredictable_branches(
            seed1 ^ seed2 ^ outer,
            seed3,
            seed1 + seed2
        );
        
        /* Modify seeds to create varying patterns */
        seed1 = (seed1 * 13 + 7) & 0xFFF;
        seed2 = (seed2 * 17 + 11) & 0xFFF;
        seed3 = (seed3 * 19 + 13) & 0xFFF;
    }
    
    /* Ensure result is used */
    printf("Accumulated result: %d\n", total_result);
    
    return total_result != 0 ? 0 : 1;
}
