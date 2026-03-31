/* test_sched_context.c - Comprehensive test for Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function with pure computation to encourage scheduling */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (c & 0xFF);
}

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t3 << (e & 3);
    int t5 = t4 - a;
    int t6 = t5 * b;
    int t7 = t6 / (c + 1);
    int t8 = t7 | d;
    int t9 = t8 & 0xFFFF;
    int t10 = t9 + e;
    int t11 = t10 * 3;
    int t12 = t11 - t1;
    int t13 = t12 + t2;
    int t14 = t13 ^ t3;
    int t15 = t14 << 2;
    int t16 = t15 >> 1;
    int t17 = t16 * 7;
    int t18 = t17 % 1023;
    int t19 = t18 + t4;
    int t20 = t19 - t5;
    int t21 = t20 * t6;
    int t22 = t21 / (t7 + 1);
    int t23 = t22 | t8;
    int t24 = t23 & t9;
    int t25 = t24 + t10;
    return t25;
}

/* Function with SSE intrinsics to trigger target-specific scheduling */
static float sse_computation(float a, float b, float c, float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Multiple dependent SSE operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, _mm_set1_ps(5.0f));
    
    /* Mix with scalar operations */
    float result[4];
    _mm_storeu_ps(result, r4);
    
    /* Create dependencies between SSE and integer ops */
    int mask = _mm_movemask_ps(r4);
    result[0] += (mask & 1) ? 1.0f : 0.0f;
    result[1] += (mask & 2) ? 2.0f : 0.0f;
    
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_computation(int a, int b, int c, volatile int* mem) {
    int result = 0;
    
    /* Initial computation */
    int t1 = a * b + c;
    
    /* Assembly barrier that the scheduler cannot see through */
    asm volatile ("" : "+r" (t1) : : "memory");
    
    /* More computation dependent on t1 */
    int t2 = t1 << 3;
    int t3 = t2 ^ b;
    
    /* Another barrier */
    asm volatile ("# barrier" : : : "memory");
    
    /* Memory operation that creates dependency */
    *mem = t3;
    int t4 = *mem + a;
    
    /* Complex assembly with multiple outputs */
    int t5, t6;
    asm volatile ("movl %2, %0\n\t"
                  "addl %3, %0\n\t"
                  "movl %0, %1"
                  : "=r"(t5), "=r"(t6)
                  : "r"(t4), "r"(c)
                  : "cc");
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    result = t5 + t6;
    return result;
}

/* Function with mixed operations and unpredictable control flow */
static int mixed_control_flow(int a, int b, int c, volatile int* flag) {
    int result = 0;
    
    /* Initial dense computation */
    int base = dense_arithmetic(a, b, c, a ^ b, c + 1);
    
    /* Unpredictable branch */
    if (*flag & 1) {
        /* Branch with vector operations */
        float f1 = (float)(base & 0xFF) / 256.0f;
        float f2 = (float)((base >> 8) & 0xFF) / 256.0f;
        result = (int)(sse_computation(f1, f2, f1 * 2.0f, f2 * 3.0f) * 1000.0f);
    } else {
        /* Alternative path with more barriers */
        volatile int mem = 0;
        result = asm_barrier_computation(base, b, c, &mem);
    }
    
    /* Loop with variable iterations */
    int loop_count = (*flag & 3) + 2;
    for (int i = 0; i < loop_count; ++i) {
        /* Inline assembly with resource constraints */
        int temp;
        asm volatile ("imull %1, %0"
                      : "=r"(temp)
                      : "r"(result), "0"(i + 1));
        result ^= temp;
        
        /* Memory clobber to prevent reordering */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int flag = argc > 4 ? atoi(argv[4]) : 0;
    
    int total = 0;
    volatile int mem_buffer[16] = {0};
    
    /* Main loop that creates multiple scheduling contexts */
    for (int iter = 0; iter < 100; ++iter) {
        /* Vary the computation based on iteration to create different contexts */
        int mode = iter % 4;
        
        switch (mode) {
            case 0: {
                /* Pure arithmetic intensive */
                int val = dense_arithmetic(
                    seed1 + iter,
                    seed2 - iter,
                    seed3 ^ iter,
                    seed1 * seed2,
                    seed3
                );
                total += pure_helper(val, seed1, seed2);
                break;
            }
            
            case 1: {
                /* SSE/Vector intensive */
                float f1 = (float)((seed1 + iter) & 0xFF) / 256.0f;
                float f2 = (float)((seed2 - iter) & 0xFF) / 256.0f;
                float fval = sse_computation(
                    f1,
                    f2,
                    f1 * 1.5f,
                    f2 * 2.5f
                );
                total += (int)(fval * 100.0f);
                break;
            }
            
            case 2: {
                /* Assembly barrier intensive */
                int val = asm_barrier_computation(
                    seed1 ^ iter,
                    seed2 + iter,
                    seed3 - iter,
                    &mem_buffer[iter % 16]
                );
                total += val;
                break;
            }
            
            case 3: {
                /* Mixed control flow */
                flag = (flag + iter) & 0xF;
                int val = mixed_control_flow(
                    seed1,
                    seed2 + iter,
                    seed3 ^ iter,
                    &flag
                );
                total += val;
                break;
            }
        }
        
        /* Occasionally insert a scheduling barrier */
        if (iter % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
        
        /* Modify seeds to create varying dependencies */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        if (iter % 3 == 0) {
            seed2 ^= total & 0xFF;
        }
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int final_result = total;
    
    /* Print to ensure all computations are observable */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
