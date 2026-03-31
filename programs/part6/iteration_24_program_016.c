/* test_sched_context.c */
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
    volatile int sink;
    int val = arr[idx];
    arr[idx] = val * 2;
    sink = arr[idx];
    return sink + idx;
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int seed) {
    int a = seed * 3;
    int b = seed + 7;
    int c = seed ^ 0x55AA;
    int d = seed >> 4;
    int e = seed << 2;
    
    /* Create many independent operations */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t2 - e;
    int t5 = t3 & t4;
    int t6 = t4 | t5;
    int t7 = t5 * t6;
    int t8 = t6 + t7;
    int t9 = t7 ^ t8;
    int t10 = t8 - t9;
    int t11 = t9 & t10;
    int t12 = t10 | t11;
    int t13 = t11 * t12;
    int t14 = t12 + t13;
    int t15 = t13 ^ t14;
    int t16 = t14 - t15;
    int t17 = t15 & t16;
    int t18 = t16 | t17;
    int t19 = t17 * t18;
    int t20 = t18 + t19;
    
    /* Mix memory operations */
    volatile int mem_sink;
    int *ptr = &t20;
    mem_sink = *ptr;
    *ptr = mem_sink + 1;
    
    return t20;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static float vector_operations(int seed) {
    __m128 vec1 = _mm_set_ps(seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f);
    __m128 vec2 = _mm_set_ps(seed * 5.0f, seed * 6.0f, seed * 7.0f, seed * 8.0f);
    
    /* Multiple vector operations to create scheduling complexity */
    __m128 res1 = _mm_add_ps(vec1, vec2);
    __m128 res2 = _mm_mul_ps(vec1, vec2);
    __m128 res3 = _mm_sub_ps(res1, res2);
    __m128 res4 = _mm_add_ps(res3, _mm_set1_ps(1.0f));
    
    /* Scalar operations mixed with vector */
    float temp[4];
    _mm_storeu_ps(temp, res4);
    
    volatile float sink;
    sink = temp[0] + temp[1] + temp[2] + temp[3];
    
    return sink;
}

/* Function with inline assembly barriers - forces state restoration */
static int assembly_barriers(int a, int b) {
    int result;
    
    /* First computation */
    asm volatile ("addl %1, %0" : "+r" (a) : "r" (b));
    
    /* Memory clobber barrier - scheduler cannot move across this */
    asm volatile ("" : : : "memory");
    
    /* Second computation */
    asm volatile ("imull %1, %0" : "+r" (a) : "r" (b));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Complex computation with multiple outputs */
    asm volatile (
        "movl %1, %%eax\n\t"
        "movl %2, %%ebx\n\t"
        "addl %%ebx, %%eax\n\t"
        "imull %%ebx, %%eax\n\t"
        "movl %%eax, %0"
        : "=r" (result)
        : "r" (a), "r" (b)
        : "%eax", "%ebx", "memory"
    );
    
    return result;
}

/* Function with unpredictable branching - creates control flow for speculative scheduling */
static int branching_pattern(int seed, int limit) {
    volatile int condition = seed;
    int result = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable branch */
        if (condition & (1 << (i % 16))) {
            /* Complex computation in taken branch */
            int a = i * seed;
            int b = seed + i;
            asm volatile ("addl %1, %0" : "+r" (a) : "r" (b));
            result += a;
        } else {
            /* Different computation in not-taken branch */
            int c = i ^ seed;
            int d = seed - i;
            asm volatile ("subl %1, %0" : "+r" (c) : "r" (d));
            result -= c;
        }
        
        /* Volatile read makes branch unpredictable */
        condition = condition * 1103515245 + 12345;
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int iterations = argc > 4 ? atoi(argv[4]) : 100;
    
    int total = 0;
    int arr[64];
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 64; ++i) {
        arr[i] = (seed1 + i) ^ seed2;
    }
    
    /* Main loop - creates multiple scheduling contexts */
    for (volatile int iter = 0; iter < iterations; ++iter) {
        /* Mix different patterns to engage various scheduler features */
        
        /* 1. Dense arithmetic - fills instruction queue */
        total += dense_arithmetic(seed1 + iter);
        
        /* 2. Vector operations - triggers target-specific scheduling hooks */
        total += (int)vector_operations(seed2 + iter);
        
        /* 3. Assembly with barriers - forces state save/restore */
        total += assembly_barriers(seed1, seed2 + iter);
        
        /* 4. Memory-dependent operations */
        total += helper_mem(arr, iter % 64);
        
        /* 5. Pure function calls - create scheduling barriers */
        total += helper_pure(seed3, iter);
        
        /* 6. Unpredictable branching - speculative scheduling contexts */
        total += branching_pattern(seed3 + iter, 8 + (iter % 16));
        
        /* Modify seeds to create varying patterns */
        seed1 = seed1 * 1664525 + 1013904223;
        seed2 = seed2 * 1103515245 + 12345;
        seed3 = seed3 * 134775813 + 1;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
