/* test_sched_context.c - Designed to trigger free_sched_context logic in GCC Haifa scheduler */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - creates scheduling barriers */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Helper with memory dependencies */
static int helper_mem(int *arr, int idx) {
    int val = arr[idx];
    arr[idx] = val * 2 + 1;
    return arr[idx] ^ val;
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = e << 2;
    int t5 = t3 - t4;
    int t6 = a * c + b * d;
    int t7 = t5 | t6;
    int t8 = t2 ^ t7;
    int t9 = t4 + t8;
    int t10 = t1 * t3;
    int t11 = t9 - t10;
    int t12 = t6 >> 1;
    int t13 = t11 ^ t12;
    int t14 = t8 + t13;
    int t15 = t14 * t7;
    int t16 = t15 - t5;
    int t17 = t16 ^ t10;
    int t18 = t17 + t12;
    int t19 = t18 * t3;
    int t20 = t19 - t14;
    
    /* Mix in memory operations */
    volatile int mem[8];
    mem[0] = t20;
    mem[1] = t15;
    mem[2] = t10;
    mem[3] = t5;
    
    int t21 = mem[0] + mem[1];
    int t22 = mem[2] * mem[3];
    int t23 = t21 ^ t22;
    
    return t20 + t23;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static float vector_ops(float a, float b, float c, float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Create dependency chain with vector ops */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, _mm_set1_ps(1.5f));
    
    /* Horizontal add pattern */
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(2, 3, 0, 1)));
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result[4] __attribute__((aligned(16)));
    _mm_store_ps(result, r4);
    
    /* Inline asm with memory clobber - creates scheduling barrier */
    asm volatile ("" : : "r"(result) : "memory");
    
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly constraints - creates complex dependencies */
static int asm_constraints(int a, int b, int c, int d) {
    int out1, out2, out3;
    
    /* Chain of dependent asm operations */
    asm volatile (
        "add %0, %1, %2\n\t"
        "mul %1, %0, %3\n\t"
        : "=r"(out1) : "r"(a), "r"(b), "r"(c) : "cc"
    );
    
    /* Another asm with different constraints */
    asm volatile (
        "sub %0, %1, %2\n\t"
        "and %1, %0, %3\n\t"
        : "=r"(out2) : "r"(out1), "r"(d), "r"(a) : "cc"
    );
    
    /* Memory barrier asm */
    asm volatile ("" : : : "memory");
    
    /* Final asm with output dependency */
    asm volatile (
        "xor %0, %1, %2\n\t"
        "orr %0, %0, %3\n\t"
        : "=r"(out3) : "r"(out2), "r"(b), "r"(c)
    );
    
    return out3;
}

/* Function with unpredictable branching - encourages speculative scheduling */
static int branching_pattern(int a, int b, volatile int *cond) {
    int result = a;
    
    /* Loop with volatile condition - prevents optimization */
    for (int i = 0; i < *cond; ++i) {
        if (i % 3 == 0) {
            result += helper_pure(result, b);
            /* Inline asm barrier in one path */
            asm volatile ("" : : : "memory");
        } else if (i % 3 == 1) {
            result ^= asm_constraints(result, b, i, *cond);
        } else {
            result *= dense_arithmetic(result, b, i, *cond, a);
        }
        
        /* Another volatile read in loop */
        volatile int temp = *cond;
        if (temp > 100) {
            result -= temp;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int loop_cond = argc > 4 ? atoi(argv[4]) : 50;
    
    int total = 0;
    int arr[16];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 16; i++) {
        arr[i] = seed1 + i * seed2;
    }
    
    /* Main loop - creates multiple scheduling contexts */
    for (int iter = 0; iter < 100; iter++) {
        volatile int local_cond = loop_cond + (iter % 10);
        
        /* Pattern 1: Vector operations (triggers target-specific scheduling) */
        float f1 = (float)(seed1 + iter);
        float f2 = (float)(seed2 - iter);
        float f3 = (float)(seed3 * iter);
        float f4 = (float)(iter);
        
        float vec_result = vector_ops(f1, f2, f3, f4);
        total += (int)vec_result;
        
        /* Pattern 2: Dense arithmetic (fills instruction queue) */
        int dense_result = dense_arithmetic(
            seed1 + iter, 
            seed2 - iter, 
            seed3 ^ iter, 
            iter * 2, 
            total & 0xFF
        );
        total ^= dense_result;
        
        /* Pattern 3: Inline assembly with constraints */
        int asm_result = asm_constraints(
            total, 
            seed1, 
            seed2 + iter, 
            seed3 - iter
        );
        total += asm_result;
        
        /* Pattern 4: Unpredictable branching */
        int branch_result = branching_pattern(total, seed2, &local_cond);
        total = helper_pure(total, branch_result);
        
        /* Pattern 5: Memory operations with helper */
        int mem_idx = (iter * 7) % 16;
        int mem_result = helper_mem(arr, mem_idx);
        total -= mem_result;
        
        /* Mix in some pure computations */
        for (int j = 0; j < 5; j++) {
            total = helper_pure(total, arr[j] + iter);
        }
        
        /* Occasionally reset with volatile write */
        if (iter % 23 == 0) {
            volatile int reset = seed3;
            total = reset + (total & 0xFFFF);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Also return as exit code for scriptable verification */
    return total & 0xFF;
}
