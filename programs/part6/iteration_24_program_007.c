/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - creates scheduling opportunities */
static int helper_pure(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (c & 0xFF);
}

/* Helper with memory operations - creates load/store dependencies */
static int helper_memops(volatile int* arr, int idx1, int idx2, int idx3) {
    int t1 = arr[idx1];
    int t2 = arr[idx2];
    arr[idx3] = t1 + t2;
    return arr[idx3] * 2;
}

/* Function with vector operations - triggers target-specific scheduling */
static float helper_vector(volatile float a, volatile float b, 
                          volatile float c, volatile float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_add_ps(v1, v2);
    __m128 v4 = _mm_mul_ps(v3, v1);
    
    float result[4];
    _mm_storeu_ps(result, v4);
    return result[0] + result[1] + result[2] + result[3];
}

/* Function with inline assembly barriers - forces scheduler state saves */
static int helper_asm_barrier(volatile int x, volatile int y, volatile int z) {
    int r1, r2, r3;
    
    /* First computation cluster */
    asm volatile ("addl %1, %0" : "=r"(r1) : "r"(x), "0"(y));
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Second computation cluster */
    asm volatile ("imull %1, %0" : "=r"(r2) : "r"(z), "0"(r1));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Third computation cluster */
    asm volatile ("xorl %1, %0" : "=r"(r3) : "r"(r2), "0"(x));
    
    return r3;
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int helper_dense_arithmetic(volatile int a, volatile int b, 
                                  volatile int c, volatile int d) {
    /* Create many independent operations to give scheduler work */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - c;
    int t5 = b << 2;
    int t6 = d >> 1;
    int t7 = t3 & 0xFF;
    int t8 = t4 | t5;
    int t9 = t6 + t7;
    int t10 = t8 * t9;
    int t11 = t1 + t2 + t3;
    int t12 = t4 - t5 - t6;
    int t13 = t7 * t8 * t9;
    int t14 = t10 ^ t11 ^ t12;
    int t15 = t13 + t14;
    int t16 = t15 << 3;
    int t17 = t16 >> 1;
    int t18 = t17 & 0x7F;
    int t19 = t18 | 0x80;
    int t20 = t19 * a;
    int t21 = t20 / (b + 1);
    int t22 = t21 ^ c;
    int t23 = t22 + d;
    int t24 = t23 * 3;
    int t25 = t24 - 7;
    
    return t25;
}

/* Function with unpredictable branching - creates control flow for speculative scheduling */
static int helper_branchy(volatile int x, volatile int limit) {
    int result = x;
    
    for (int i = 0; i < limit; i++) {
        /* Unpredictable branch based on computation */
        if ((result & 1) == 0) {
            result = helper_pure(result, i, x);
        } else {
            /* Inline assembly in one path */
            int temp;
            asm volatile ("movl %1, %0\n\t"
                         "addl $1, %0" 
                         : "=r"(temp) : "r"(result));
            result = temp * 2;
        }
        
        /* Another branch point */
        if (i % 3 == 0) {
            result ^= 0xABCD;
        } else if (i % 3 == 1) {
            result += 0x1234;
        } else {
            result *= 3;
        }
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 98765;
    
    int total = 0;
    volatile int iter_count = 100;  /* Force multiple scheduler contexts */
    
    /* Array for memory operations */
    volatile int mem_array[64];
    for (int i = 0; i < 64; i++) {
        mem_array[i] = i * seed1;
    }
    
    /* Main loop - each iteration may create different scheduling contexts */
    for (volatile int iter = 0; iter < iter_count; iter++) {
        int idx = iter & 63;
        
        /* Mix different types of computations to engage various scheduler features */
        
        /* 1. Pure computation - basic scheduling */
        total += helper_pure(seed1 + iter, seed2, seed3);
        
        /* 2. Memory operations - load/store scheduling */
        total += helper_memops((volatile int*)mem_array, 
                              idx, (idx + 1) & 63, (idx + 2) & 63);
        
        /* 3. Vector operations - target-specific scheduling hooks */
        if (iter % 4 == 0) {
            float fval = helper_vector(seed1 * 0.1f, seed2 * 0.2f,
                                      seed3 * 0.3f, seed4 * 0.4f);
            total += (int)fval;
        }
        
        /* 4. Inline assembly with barriers - forces state save/restore */
        total += helper_asm_barrier(seed1 ^ iter, seed2, seed3);
        
        /* 5. Dense arithmetic - fills instruction queue */
        if (iter % 3 == 0) {
            total += helper_dense_arithmetic(seed1, seed2 + iter, 
                                           seed3, seed4 - iter);
        }
        
        /* 6. Branchy code - speculative scheduling opportunities */
        total += helper_branchy(seed4, (iter % 8) + 2);
        
        /* Modify seeds to create varying patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
