/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function with pure computation - creates scheduling region */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (b * c) - (a * c);
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to fill scheduler structures */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a - d;
    t5 = b + c;
    t6 = t4 * t5;
    t7 = t3 | t6;
    t8 = a * c;
    t9 = b * d;
    t10 = t8 - t9;
    t11 = t7 + t10;
    t12 = a ^ d;
    t13 = b ^ c;
    t14 = t12 & t13;
    t15 = t11 * t14;
    t16 = t1 + t2 + t3;
    t17 = t4 * t5 * t6;
    t18 = t7 - t8 - t9;
    t19 = t10 ^ t11 ^ t12;
    t20 = t13 | t14 | t15;
    
    /* Create dependencies to force ordering */
    t1 = t20 + t19;
    t2 = t18 * t17;
    t3 = t16 - t15;
    t4 = t14 / (t13 + 1);
    t5 = t12 ^ t11;
    
    return t1 + t2 + t3 + t4 + t5;
}

/* Function with inline assembly barriers - creates scheduling boundaries */
static int asm_barrier_test(int a, int b, int c) {
    int result1, result2, result3;
    
    /* First computation with memory barrier */
    asm volatile (
        "mov %1, %%eax\n\t"
        "add %2, %%eax\n\t"
        "mov %%eax, %0\n\t"
        : "=r" (result1)
        : "r" (a), "r" (b)
        : "%eax", "memory"
    );
    
    /* Second computation - scheduler may try to move across barrier */
    result2 = pure_helper(a, b, c);
    
    /* Another barrier */
    asm volatile (
        "mov %1, %%ebx\n\t"
        "imul %2, %%ebx\n\t"
        "mov %%ebx, %0\n\t"
        : "=r" (result3)
        : "r" (result2), "r" (c)
        : "%ebx", "memory"
    );
    
    return result1 + result3;
}

/* Function with SSE intrinsics - triggers target-specific scheduling */
static __m128 sse_computation(__m128 a, __m128 b, __m128 c) {
    __m128 t1, t2, t3, t4, t5, t6;
    
    /* Mix of SSE operations to engage vector scheduling */
    t1 = _mm_add_ps(a, b);
    t2 = _mm_mul_ps(b, c);
    t3 = _mm_sub_ps(a, c);
    t4 = _mm_add_ps(t1, t2);
    t5 = _mm_mul_ps(t3, t4);
    t6 = _mm_add_ps(t5, _mm_set1_ps(1.0f));
    
    /* Create dependency chain */
    t1 = _mm_add_ps(t6, a);
    t2 = _mm_mul_ps(t1, b);
    t3 = _mm_sub_ps(t2, c);
    
    return t3;
}

/* Function with unpredictable branching - encourages speculative scheduling */
static int branching_test(int a, int b, int c, volatile int* control) {
    int result = 0;
    
    /* Loop with volatile condition - scheduler can't predict trip count */
    for (int i = 0; i < *control; ++i) {
        /* Branch with data-dependent condition */
        if ((a * i) > (b + c)) {
            result += pure_helper(a, i, b);
        } else {
            result -= dense_arithmetic(b, i, c, a);
        }
        
        /* Mix in some inline assembly */
        int temp;
        asm volatile (
            "mov %1, %%eax\n\t"
            "add $1, %%eax\n\t"
            "mov %%eax, %0\n\t"
            : "=r" (temp)
            : "r" (result)
            : "%eax"
        );
        result = temp;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int loop_control = argc > 4 ? atoi(argv[4]) : 100;
    
    int total = 0;
    
    /* Main loop - creates multiple scheduling contexts */
    for (int iter = 0; iter < loop_control; ++iter) {
        /* Vary the inputs to create different scheduling patterns */
        int a = seed1 + iter;
        int b = seed2 - iter;
        int c = seed3 * (iter + 1);
        
        /* Call different test functions to exercise various scheduler behaviors */
        
        /* 1. Test with dense arithmetic - fills instruction queue */
        total += dense_arithmetic(a, b, c, iter);
        
        /* 2. Test with assembly barriers - may cause state save/restore */
        total += asm_barrier_test(b, c, a);
        
        /* 3. Test with branching and volatile control - speculative scheduling */
        volatile int control = (iter % 10) + 5;
        total += branching_test(c, a, b, &control);
        
        /* 4. Test with SSE intrinsics - triggers target-specific scheduling hooks */
        if (iter % 3 == 0) {
            __m128 vec_a = _mm_set_ps(a, b, c, iter);
            __m128 vec_b = _mm_set_ps(b, c, a, iter);
            __m128 vec_c = _mm_set_ps(c, a, b, iter);
            
            __m128 vec_result = sse_computation(vec_a, vec_b, vec_c);
            
            /* Extract some value from vector */
            float result_array[4];
            _mm_store_ps(result_array, vec_result);
            total += (int)result_array[0] + (int)result_array[1] +
                     (int)result_array[2] + (int)result_array[3];
        }
        
        /* 5. Mix in some pure function calls */
        for (int j = 0; j < 5; ++j) {
            total += pure_helper(a + j, b - j, c * j);
        }
        
        /* Create memory operations to introduce load/store dependencies */
        int* mem_buffer = (int*)malloc(16 * sizeof(int));
        if (mem_buffer) {
            for (int j = 0; j < 16; ++j) {
                mem_buffer[j] = a + b + c + j;
            }
            
            /* Use the memory in computations */
            int mem_sum = 0;
            for (int j = 0; j < 16; ++j) {
                mem_sum += mem_buffer[j];
            }
            total += mem_sum;
            
            free(mem_buffer);
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
