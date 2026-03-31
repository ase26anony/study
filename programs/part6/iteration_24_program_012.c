/* test_sched_context.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - encourages speculative scheduling */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Helper with memory operations - creates load/store dependencies */
static int helper_memops(int *arr, int idx1, int idx2, int val) {
    arr[idx1] = val;
    int t1 = arr[idx2];
    arr[idx2] = t1 * 2;
    return arr[idx1] + arr[idx2];
}

/* Function with dense independent arithmetic operations - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a - e;
    int t5 = b << 2;
    int t6 = c >> 1;
    int t7 = d | e;
    int t8 = t3 & t4;
    int t9 = t5 + t6;
    int t10 = t7 * t8;
    int t11 = t9 - t10;
    int t12 = a * b * c;
    int t13 = d + e + t11;
    int t14 = t12 ^ t13;
    int t15 = t14 << 3;
    int t16 = t15 >> 1;
    int t17 = t16 & 0xFF;
    int t18 = t17 * 17;
    int t19 = t18 + 12345;
    int t20 = t19 - 54321;
    return t20;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling hooks */
static float vector_operations(float f1, float f2, float f3, float f4) {
    __m128 v1 = _mm_set_ps(f1, f2, f3, f4);
    __m128 v2 = _mm_set1_ps(2.0f);
    __m128 v3 = _mm_set1_ps(3.0f);
    
    /* Create dependency chain with vector ops */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, r1);
    
    /* Horizontal add pattern */
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(2, 3, 0, 1)));
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, r4);
    return result;
}

/* Function with inline assembly barriers - forces scheduler backtracking */
static int asm_barrier_ops(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation chain */
    asm volatile ("addl %1, %0" : "=r"(res1) : "r"(a), "0"(b));
    
    /* Memory clobber barrier - scheduler may try to move across this */
    asm volatile ("" : : : "memory");
    
    /* Second computation dependent on first */
    asm volatile ("imull %1, %0" : "=r"(res2) : "r"(res1), "0"(c));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Final computation */
    asm volatile ("xorl %1, %0" : "=r"(res3) : "r"(res2), "0"(res1));
    
    return res3;
}

/* Complex loop with unpredictable branching - creates multiple scheduling contexts */
static int branching_pattern(volatile int limit, int seed) {
    int total = 0;
    volatile int counter = 0;
    
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable branch based on volatile */
        if (counter % 7 < 3) {
            /* Path with function call - scheduling barrier */
            total += helper_pure(total, seed + i);
            
            /* Inline asm with dependencies */
            int tmp;
            asm volatile ("movl %1, %0\n\t"
                         "addl $1, %0\n\t"
                         "imull %2, %0" 
                         : "=r"(tmp) 
                         : "r"(total), "r"(i));
            total = tmp;
        } else {
            /* Alternative path with memory ops */
            int arr[4] = {total, seed, i, limit};
            total += helper_memops(arr, i % 4, (i + 1) % 4, total);
        }
        
        /* Volatile update affects loop control */
        counter++;
        
        /* Mix in some vector operations occasionally */
        if (i % 5 == 0) {
            float ftotal = (float)total;
            ftotal += vector_operations(ftotal, ftotal * 0.5f, 
                                       ftotal * 0.25f, ftotal * 0.125f);
            total = (int)ftotal;
        }
    }
    
    return total;
}

/* Main driver that creates multiple scheduling contexts */
int main(int argc, char *argv[]) {
    /* Use argv for volatile seed to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    
    int total = 0;
    
    /* Outer loop to increase chance of context creation */
    for (int outer = 0; outer < 100; ++outer) {
        volatile int iter = (outer % 10) + 5;  /* Varying trip count */
        
        /* 1. Branching pattern - creates control flow for context saves */
        total += branching_pattern(iter, seed1 + outer);
        
        /* 2. Dense arithmetic - fills instruction queue */
        total += dense_arithmetic(total, seed2, outer, iter, seed3);
        
        /* 3. Assembly barrier operations - forces state restoration */
        total += asm_barrier_ops(total, seed1, seed2);
        
        /* 4. Vector operations - triggers target-specific hooks */
        float ftotal = (float)total;
        ftotal += vector_operations(ftotal, ftotal * 0.3f, 
                                   ftotal * 0.7f, ftotal * 1.1f);
        total = (int)ftotal;
        
        /* 5. Mixed operations in tight loop */
        for (int inner = 0; inner < (iter % 8) + 2; ++inner) {
            /* Create artificial dependencies */
            int a = total + inner;
            int b = seed3 + outer;
            int c = a ^ b;
            
            /* Inline asm with resource constraints */
            int res;
            asm volatile ("addl %%ebx, %%eax\n\t"
                         "imull %%ecx, %%eax\n\t"
                         "xorl %%edx, %%eax"
                         : "=a"(res)
                         : "a"(a), "b"(b), "c"(c), "d"(inner)
                         : "cc");
            
            total = res;
            
            /* Memory operation between dependent chains */
            volatile int mem = total;
            total = mem + inner;
        }
    }
    
    /* Make result observable */
    printf("Result: %d\n", total);
    return total & 0xFF;  /* Return non-zero to indicate execution */
}
