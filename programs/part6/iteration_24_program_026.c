/* test_sched_context.c
 * Designed to trigger free_sched_context logic in GCC's Haifa scheduler
 * Compile with: gcc -O3 -funroll-loops -fschedule-insns2 -march=native -c test_sched_context.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */

/* Helper functions to create scheduling complexity */

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, volatile int iter) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Create many independent operations to fill scheduler queues */
        t1 = a + b;
        t2 = c * d;
        t3 = t1 ^ t2;
        t4 = a * c + b;
        t5 = b * d - a;
        t6 = t3 & t4;
        t7 = t5 | t6;
        t8 = t1 * t2 + t3;
        t9 = t4 - t5 * t6;
        t10 = t7 ^ t8 & t9;
        
        t11 = t1 + t2 + t3;
        t12 = t4 * t5 - t6;
        t13 = t7 ^ t8 | t9;
        t14 = t10 + t11 * t12;
        t15 = t13 - t14 & t10;
        t16 = t11 * t12 + t13;
        t17 = t14 ^ t15 | t16;
        t18 = t9 + t10 * t11;
        t19 = t12 - t13 & t14;
        t20 = t15 | t16 ^ t17;
        
        /* Mix in memory operations */
        volatile int mem_var = t18;
        t19 = mem_var + t19;
        
        /* Final computation with dependencies */
        result += ((t1 + t20) * (t2 - t19)) ^ (t3 * t18);
    }
    
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(int a, int b, volatile int cond) {
    int result = a;
    
    for (int i = 0; i < 10; ++i) {
        /* Initial computation */
        int tmp1 = a * b + i;
        int tmp2 = a ^ b - i;
        
        /* Assembly barrier that scheduler might try to move across */
        asm volatile ("" : "+r" (tmp1), "+r" (tmp2) : : "memory");
        
        /* Dependent computations after barrier */
        int tmp3 = tmp1 * tmp2;
        int tmp4 = tmp1 + tmp2;
        
        /* Another barrier */
        asm volatile ("# barrier %0, %1" : "+r" (tmp3), "+r" (tmp4));
        
        /* Branch with unpredictable outcome */
        if (cond & (1 << i)) {
            /* Complex path with more barriers */
            asm volatile ("mov %0, %0" : "+r" (tmp3));
            result += tmp3 * tmp4;
        } else {
            /* Alternative path */
            asm volatile ("" : : : "memory");
            result -= tmp3 + tmp4;
        }
        
        /* Final barrier in loop */
        asm volatile ("# loop end" : : : "memory");
    }
    
    return result;
}

/* Function using SSE intrinsics to trigger target-specific scheduling */
static int sse_vector_ops(float a, float b, float c, float d, volatile int iter) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(b, c, d, a);
    __m128 vec3 = _mm_set_ps(c, d, a, b);
    __m128 result_vec = _mm_setzero_ps();
    
    for (int i = 0; i < iter; ++i) {
        /* Mix of SSE operations */
        __m128 temp1 = _mm_add_ps(vec1, vec2);
        __m128 temp2 = _mm_mul_ps(vec2, vec3);
        
        /* Conditional execution path */
        if (i & 1) {
            temp1 = _mm_sub_ps(temp1, vec3);
            temp2 = _mm_add_ps(temp2, vec1);
        } else {
            temp1 = _mm_mul_ps(temp1, _mm_set1_ps(1.5f));
        }
        
        /* More operations with dependencies */
        __m128 temp3 = _mm_add_ps(temp1, temp2);
        temp3 = _mm_mul_ps(temp3, _mm_set1_ps(0.5f));
        
        result_vec = _mm_add_ps(result_vec, temp3);
        
        /* Modify vectors for next iteration */
        vec1 = _mm_shuffle_ps(vec1, vec1, _MM_SHUFFLE(2, 3, 0, 1));
        vec2 = _mm_add_ps(vec2, _mm_set1_ps(0.1f));
    }
    
    /* Extract result */
    float result[4];
    _mm_store_ps(result, result_vec);
    return (int)(result[0] + result[1] + result[2] + result[3]);
}

/* Function with complex control flow and mixed operations */
static int mixed_control_flow(int base, volatile int mod) {
    int arr[16];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        arr[i] = base + i * mod;
    }
    
    /* Complex loop with multiple exit points */
    for (int i = 0; i < 100; ++i) {
        int idx = i & 0xF;
        
        /* Inline assembly with register constraints */
        int val;
        asm volatile ("mov %1, %0\n\t"
                      "add $1, %0"
                      : "=r" (val)
                      : "r" (arr[idx]));
        
        /* Memory operation that creates dependency */
        arr[idx] = val;
        
        /* Branch based on computation */
        if (val % 3 == 0) {
            /* Path with more computations */
            int tmp1 = val * val;
            int tmp2 = tmp1 + arr[(idx + 1) & 0xF];
            sum += tmp2;
            
            /* Inline asm that scheduler might speculate across */
            asm volatile ("# speculative point %0" : "+r" (tmp2));
            
            if (tmp2 & 1) {
                sum -= arr[(idx + 2) & 0xF];
            }
        } else if (val % 5 == 0) {
            /* Alternative path */
            sum -= val;
            
            /* Force memory synchronization */
            asm volatile ("" : : : "memory");
            
            /* Additional dependent operations */
            for (int j = 0; j < 3; ++j) {
                sum += arr[(idx + j) & 0xF] * j;
            }
        } else {
            /* Default path with simple operation */
            sum ^= val;
        }
        
        /* Loop-carried dependency */
        arr[idx] = sum & 0xFF;
    }
    
    return sum;
}

/* Main function that creates multiple scheduling contexts */
int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 456;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the iteration counts to create different scheduling patterns */
        volatile int iter1 = (seed1 + outer) % 50 + 10;
        volatile int iter2 = (seed2 + outer) % 30 + 5;
        
        /* Call different helper functions to create varied scheduling contexts */
        total += dense_arithmetic(seed1 + outer, seed2, seed3, seed4, iter1);
        
        total ^= asm_barrier_ops(seed2 + outer, seed3, seed4);
        
        total += sse_vector_ops(seed1 * 0.1f, seed2 * 0.2f, 
                               seed3 * 0.3f, seed4 * 0.4f, iter2);
        
        total -= mixed_control_flow(seed3 + outer, seed4);
        
        /* Occasionally add a complex branching pattern */
        if (outer % 7 == 0) {
            volatile int cond = seed1 ^ outer;
            for (int inner = 0; inner < 5; ++inner) {
                /* Nested loop with inline asm */
                int tmp = seed2 + inner;
                asm volatile ("imul %1, %0" : "+r" (tmp) : "r" (seed3));
                
                if (cond & (1 << inner)) {
                    total += tmp * seed4;
                } else {
                    total -= tmp / (seed4 + 1);
                }
                
                /* Memory clobber that might cause scheduler to save/restore */
                asm volatile ("" : : : "memory");
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
