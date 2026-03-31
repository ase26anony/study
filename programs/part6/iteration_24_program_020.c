/* test_sched_context.c
 * Designed to trigger free_sched_context uncovered lines in haifa-sched.cc
 * Compile with: gcc -O3 -funroll-loops -fschedule-insns2 -march=native -mtune=native test_sched_context.c -o test_sched
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling complexity */

/* Function 1: Dense arithmetic sequence to fill instruction queue */
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
        t8 = t2 + t7;
        t9 = t1 * t8;
        t10 = t9 >> 2;
        t11 = t10 * 3;
        t12 = t11 + t4;
        t13 = t12 ^ t7;
        t14 = t13 * 7;
        t15 = t14 - t8;
        t16 = t15 & 0xFF;
        t17 = t16 * t9;
        t18 = t17 / (t10 + 1);
        t19 = t18 + t11;
        t20 = t19 * t14;
        
        result += t20;
        
        /* Introduce data dependencies for next iteration */
        a = t20 & 0x7F;
        b = (t19 + 1) & 0x3F;
        c = (t18 * 2) & 0x1F;
        d = (t17 + 3) & 0xF;
    }
    
    return result;
}

/* Function 2: Vector operations to trigger target-specific scheduling hooks */
static __m128 vector_ops(volatile int seed1, volatile int seed2) {
    __m128 v1, v2, v3, v4, v5, v6, v7, v8;
    float arr1[4] = {seed1 * 1.0f, seed1 * 2.0f, seed1 * 3.0f, seed1 * 4.0f};
    float arr2[4] = {seed2 * 5.0f, seed2 * 6.0f, seed2 * 7.0f, seed2 * 8.0f};
    
    v1 = _mm_loadu_ps(arr1);
    v2 = _mm_loadu_ps(arr2);
    
    /* Create chain of vector operations */
    v3 = _mm_add_ps(v1, v2);
    v4 = _mm_mul_ps(v1, v2);
    v5 = _mm_sub_ps(v3, v4);
    v6 = _mm_add_ps(v4, v5);
    v7 = _mm_mul_ps(v5, v6);
    v8 = _mm_add_ps(v6, v7);
    
    /* Inline assembly with SSE registers to ensure scheduler engagement */
    asm volatile (
        "addps %1, %0\n\t"
        "mulps %0, %0\n\t"
        : "+x"(v8)
        : "x"(v7)
        : /* No clobbers - let compiler manage */
    );
    
    return v8;
}

/* Function 3: Mixed operations with memory barriers */
static int mixed_with_barriers(int a, int b, volatile int* ptr) {
    int local1, local2, local3, local4;
    
    /* Initial computation */
    local1 = a * b + 37;
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    local2 = local1 * 2 - b;
    
    /* Another barrier */
    asm volatile ("# barrier" : : : "memory");
    
    /* More computation with memory access */
    *ptr = local2;
    local3 = *ptr + a;
    
    /* Complex inline assembly with dependencies */
    asm volatile (
        "imull %1, %0\n\t"
        "addl $0x1234, %0\n\t"
        : "+r"(local3)
        : "r"(local1)
        : "cc"
    );
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    local4 = local3 ^ (a + b);
    
    return local4;
}

/* Function 4: Branching with unpredictable control flow */
static int branching_pattern(volatile int limit) {
    int sum = 0;
    int arr[16];
    
    /* Initialize array */
    for (int i = 0; i < 16; i++) {
        arr[i] = i * (limit & 0xF);
    }
    
    /* Loop with condition that's hard to predict */
    for (int i = 0; i < limit; i++) {
        /* Branch based on complex condition */
        if ((i ^ (limit >> 2)) & 1) {
            /* Path 1: More arithmetic */
            sum += arr[i & 0xF] * 3;
            
            /* Inline assembly in one branch only */
            asm volatile (
                "movl %1, %%eax\n\t"
                "leal (%%eax,%%eax,2), %0\n\t"
                : "=r"(arr[i & 0xF])
                : "r"(sum)
                : "%eax"
            );
        } else {
            /* Path 2: Different operations */
            sum -= arr[(i + 1) & 0xF] / 2;
            
            /* Memory operation in this branch */
            volatile int temp = arr[(i + 1) & 0xF];
            arr[(i + 1) & 0xF] = temp ^ sum;
        }
        
        /* Additional computation after branch */
        sum = (sum * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return sum;
}

/* Function 5: Nested loops with function calls */
static int nested_loop_complexity(int base, volatile int outer_iter) {
    int total = 0;
    
    for (int i = 0; i < outer_iter; i++) {
        int inner_iter = (base + i) & 0x7;
        
        for (int j = 0; j < inner_iter; j++) {
            /* Function call acts as scheduling barrier */
            total += dense_arithmetic(base, i, j, total, 2);
            
            /* Insert inline assembly between calls */
            asm volatile (
                "addl %1, %0\n\t"
                "rorl $3, %0\n\t"
                : "+r"(total)
                : "r"(j)
                : "cc"
            );
        }
        
        /* Branch with different paths */
        if (i & 1) {
            total ^= (base * i);
        } else {
            total |= (base + i);
        }
    }
    
    return total;
}

/* Main function that orchestrates all patterns */
int main(int argc, char** argv) {
    volatile int seed1, seed2, seed3;
    int result = 0;
    
    /* Initialize seeds from argv to prevent constant propagation */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    volatile int* mem_ptr = (volatile int*)malloc(sizeof(int));
    *mem_ptr = seed1;
    
    /* Execute multiple iterations with different scheduling patterns */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* Pattern 1: Dense arithmetic to fill instruction queues */
        result += dense_arithmetic(seed1 + iteration, 
                                  seed2 - iteration, 
                                  seed3 ^ iteration, 
                                  result & 0xFF, 
                                  (iteration % 5) + 3);
        
        /* Pattern 2: Vector operations for target-specific scheduling */
        __m128 vec_result = vector_ops(seed1 + result, seed2 - iteration);
        float vec_sum[4];
        _mm_storeu_ps(vec_sum, vec_result);
        result += (int)(vec_sum[0] + vec_sum[1] + vec_sum[2] + vec_sum[3]);
        
        /* Pattern 3: Mixed operations with memory barriers */
        result ^= mixed_with_barriers(seed3, result, mem_ptr);
        
        /* Pattern 4: Branching with unpredictable control flow */
        result += branching_pattern((seed1 + iteration) & 0x1F);
        
        /* Pattern 5: Nested loops with function calls */
        if (iteration % 3 == 0) {
            result -= nested_loop_complexity(seed2, (iteration % 4) + 2);
        }
        
        /* Modify seeds to create varying patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed3 = (seed3 * 214013 + 2531011) & 0x7FFFFFFF;
    }
    
    free((void*)mem_ptr);
    
    /* Ensure result is used */
    printf("Final result: %d\n", result);
    
    return result & 0xFF;
}
