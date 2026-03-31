/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation - creates scheduling opportunities */
static int pure_helper(int a, int b, int c) {
    int t1 = a * b + c;
    int t2 = (a ^ b) | c;
    int t3 = t1 * t2 - a;
    int t4 = t3 >> 4;
    int t5 = t4 & 0xFF;
    return t5 * b + c;
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    /* Create many independent operations to give scheduler work */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = a * c + b * d;
    int t5 = t3 & t4;
    int t6 = t1 | t2;
    int t7 = t4 - t3;
    int t8 = t5 * t6;
    int t9 = t7 >> 2;
    int t10 = t8 & 0xFFFF;
    int t11 = t9 * t10;
    int t12 = t11 + a - b;
    int t13 = t12 * c / (d ? d : 1);
    int t14 = t13 ^ t11;
    int t15 = t14 | t12;
    int t16 = t15 & t13;
    int t17 = t16 << 3;
    int t18 = t17 - t14;
    int t19 = t18 * t15;
    int t20 = t19 >> 1;
    
    /* Mix in some memory operations */
    volatile int mem1 = t20;
    int mem2 = mem1;
    
    return t20 + mem2 + t1 + t2 + t3;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static float vector_operations(float a, float b, float c, float d) {
    __m128 v1 = _mm_set_ps(a, b, c, d);
    __m128 v2 = _mm_set_ps(d, c, b, a);
    __m128 v3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Chain of vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, _mm_set1_ps(5.0f));
    
    /* Horizontal add pattern - often requires careful scheduling */
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(2, 3, 0, 1)));
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, r4);
    return result;
}

/* Function with inline assembly barriers - forces state save/restore */
static int asm_barrier_sequence(int a, int b, int c) {
    int r1, r2, r3, r4;
    
    /* Sequence with inline asm creating scheduling barriers */
    asm volatile ("addl %1, %0" : "=r"(r1) : "r"(a), "0"(b));
    
    /* Memory clobber acts as scheduling barrier */
    asm volatile ("# Memory barrier" : : : "memory");
    
    asm volatile ("imull %1, %0" : "=r"(r2) : "r"(c), "0"(r1));
    
    /* Another barrier */
    asm volatile ("# Another barrier" : : : "memory");
    
    asm volatile ("xorl %1, %0" : "=r"(r3) : "r"(r2), "0"(a));
    
    return r3;
}

/* Function with unpredictable branching - creates control flow for speculative scheduling */
static int branching_pattern(int a, int b, volatile int* control) {
    int result = a;
    
    /* Loop with volatile condition prevents optimization */
    for (int i = 0; i < *control; ++i) {
        /* Branch with hard-to-predict outcome */
        if ((a ^ i) & 1) {
            result += pure_helper(b, i, result);
        } else {
            result -= asm_barrier_sequence(i, b, result);
        }
        
        /* Small inline asm to create micro-scheduling decisions */
        asm volatile ("# Loop body marker %0" : : "r"(i));
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile int loop_control = argc > 4 ? atoi(argv[4]) : 10;
    
    int total = 0;
    float ftotal = 0.0f;
    
    /* Main loop that exercises different scheduling patterns */
    for (int iter = 0; iter < 100; ++iter) {
        /* Vary the control flow to trigger different scheduler behaviors */
        volatile int dynamic_control = (iter % loop_control) + 1;
        
        /* 1. Exercise dense arithmetic scheduler */
        total += dense_arithmetic(seed1 + iter, seed2 - iter, 
                                 seed3 ^ iter, iter * 3);
        
        /* 2. Exercise vector operations with target-specific scheduling */
        ftotal += vector_operations(seed1 * 0.1f, seed2 * 0.2f,
                                   seed3 * 0.3f, iter * 0.4f);
        
        /* 3. Exercise branching with speculative scheduling opportunities */
        total += branching_pattern(total, seed2 + iter, &dynamic_control);
        
        /* 4. Mix in pure computations */
        total += pure_helper(total, seed3, iter);
        
        /* 5. Exercise assembly barrier sequences */
        total += asm_barrier_sequence(total, seed1, seed2);
        
        /* Occasionally reset to create new scheduling contexts */
        if (iter % 25 == 0) {
            /* Force a scheduling boundary with volatile and inline asm */
            asm volatile ("# Major iteration boundary %0" : : "r"(iter));
            volatile int reset_marker = iter;
            total += reset_marker;
        }
    }
    
    /* Mix in float results */
    total += (int)ftotal;
    
    /* Ensure result is used */
    printf("Result: %d (seeds: %d, %d, %d)\n", total, seed1, seed2, seed3);
    
    return total & 0xFF;
}
