/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - scheduler may try to move it */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Helper with memory side effects - acts as scheduling barrier */
static int helper_volatile(volatile int *a, volatile int *b) {
    int tmp = *a;
    *b = tmp + 1;
    return tmp * (*b);
}

/* Function with vector operations to trigger target-specific scheduling */
void vector_ops(volatile int seed) {
    float arr1[4] __attribute__((aligned(16)));
    float arr2[4] __attribute__((aligned(16)));
    
    for (int i = 0; i < 4; i++) {
        arr1[i] = (seed + i) * 1.5f;
        arr2[i] = (seed - i) * 0.75f;
    }
    
    __m128 vec1 = _mm_load_ps(arr1);
    __m128 vec2 = _mm_load_ps(arr2);
    
    /* Mix of vector operations to create complex scheduling constraints */
    __m128 res1 = _mm_add_ps(vec1, vec2);
    __m128 res2 = _mm_mul_ps(vec1, vec2);
    __m128 res3 = _mm_sub_ps(res1, res2);
    
    /* Horizontal add pattern - creates dependencies */
    res3 = _mm_add_ps(res3, _mm_shuffle_ps(res3, res3, _MM_SHUFFLE(2, 3, 0, 1)));
    res3 = _mm_add_ps(res3, _mm_shuffle_ps(res3, res3, _MM_SHUFFLE(1, 0, 3, 2)));
    
    _mm_store_ps(arr1, res3);
    
    /* Use the result to prevent dead code elimination */
    volatile float sink __attribute__((unused)) = arr1[0];
}

/* Function with dense arithmetic operations to fill instruction queue */
int dense_arithmetic(volatile int seed1, volatile int seed2) {
    int a = seed1;
    int b = seed2;
    int c = a ^ b;
    int d = a & b;
    int e = a | b;
    int f = a + b;
    int g = a - b;
    int h = a * b;
    
    /* Create a web of dependent operations */
    int t1 = a + c;
    int t2 = b + d;
    int t3 = c + e;
    int t4 = d + f;
    int t5 = e + g;
    int t6 = f + h;
    int t7 = g + a;
    int t8 = h + b;
    
    /* More independent operations to increase instruction count */
    int u1 = t1 * t2;
    int u2 = t3 * t4;
    int u3 = t5 * t6;
    int u4 = t7 * t8;
    int u5 = t1 ^ t3;
    int u6 = t2 ^ t4;
    int u7 = t5 ^ t7;
    int u8 = t6 ^ t8;
    
    /* Cross dependencies */
    int v1 = u1 + u5;
    int v2 = u2 + u6;
    int v3 = u3 + u7;
    int v4 = u4 + u8;
    
    /* Final mix */
    return v1 + v2 + v3 + v4 + u1 + u2 + u3 + u4;
}

/* Function with inline assembly barriers to force state save/restore */
int asm_barriers(volatile int a, volatile int b) {
    int res1, res2, res3;
    
    /* First computation cluster */
    asm volatile ("addl %1, %0" : "=r" (res1) : "r" (a), "0" (b));
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Second computation cluster with dependencies */
    asm volatile ("imull %1, %0" : "=r" (res2) : "r" (res1), "0" (a));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Third cluster */
    asm volatile ("xorl %1, %0" : "=r" (res3) : "r" (res2), "0" (b));
    
    return res1 + res2 + res3;
}

/* Function with unpredictable branching for speculative scheduling */
int branching_pattern(volatile int seed, volatile int limit) {
    int total = 0;
    
    /* Loop with volatile limit prevents unrolling */
    for (int i = 0; i < limit; i++) {
        /* Unpredictable condition */
        if ((seed ^ i) & 1) {
            /* Branch target 1: mixed operations */
            int a = seed + i;
            int b = seed - i;
            total += helper_pure(a, b);
            
            /* Inline asm with register constraints */
            int tmp;
            asm volatile ("movl %1, %0\n\t"
                         "addl $1, %0" 
                         : "=r" (tmp) 
                         : "r" (total));
            total = tmp;
        } else {
            /* Branch target 2: different operation mix */
            int a = seed * i;
            int b = seed ^ i;
            
            /* Memory operations create scheduling constraints */
            volatile int mem1 = a;
            volatile int mem2 = b;
            total += helper_volatile(&mem1, &mem2);
        }
        
        /* Small pure function call - scheduler may try to move/speculate */
        total += helper_pure(total, i);
    }
    
    return total;
}

/* Main function that combines all patterns */
int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int limit = argc > 4 ? atoi(argv[4]) : 100;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of scheduling context creation */
    for (int iter = 0; iter < 50; iter++) {
        /* Mix different scheduling patterns in each iteration */
        
        /* 1. Vector operations for target-specific scheduling */
        vector_ops(seed1 + iter);
        
        /* 2. Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(seed1 + iter, seed2 + iter);
        
        /* 3. Inline assembly with barriers */
        total += asm_barriers(seed2 + iter, seed3 + iter);
        
        /* 4. Branching pattern for speculative scheduling */
        total += branching_pattern(seed3 + iter, (limit + iter) % 50 + 10);
        
        /* 5. Direct complex computation block */
        {
            /* Create many local variables for scheduler to track */
            int a = seed1 + total;
            int b = seed2 + iter;
            int c = seed3 ^ iter;
            
            /* Long dependency chain */
            int x1 = a * b + c;
            int x2 = b * c + a;
            int x3 = c * a + b;
            int x4 = x1 ^ x2 ^ x3;
            int x5 = (x1 & x2) | (x2 & x3) | (x3 & x1);
            int x6 = x4 * x5;
            int x7 = x6 - x4 + x5;
            int x8 = x7 * 31 + 17;
            
            total += x8;
        }
        
        /* Occasionally call helper with volatile args */
        if (iter % 7 == 0) {
            volatile int v1 = seed1 + iter;
            volatile int v2 = seed2 + iter;
            total += helper_volatile(&v1, &v2);
        }
    }
    
    /* Use the result to prevent optimization */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
