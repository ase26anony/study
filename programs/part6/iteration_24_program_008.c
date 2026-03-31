/* test_sched_context.c - Comprehensive test for Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function that creates arithmetic dependencies */
static int helper_arithmetic(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b * c;
    int t3 = t1 ^ t2;
    int t4 = t3 - a;
    int t5 = t4 * 7;
    int t6 = t5 >> 2;
    int t7 = t6 & 0xFF;
    int t8 = t7 | c;
    int t9 = t8 * 3;
    return t9;
}

/* Function with dense independent operations to fill instruction queue */
static int dense_operations(volatile int seed) {
    int a = seed + 1;
    int b = seed * 2;
    int c = seed - 5;
    int d = seed ^ 0x1234;
    int e = seed * 3;
    int f = seed >> 4;
    
    /* Create many independent operations */
    int r1 = a + b;
    int r2 = c * d;
    int r3 = e ^ f;
    int r4 = a * c;
    int r5 = b + d;
    int r6 = e - f;
    int r7 = r1 * r2;
    int r8 = r3 + r4;
    int r9 = r5 ^ r6;
    int r10 = r7 - r8;
    int r11 = r9 * 2;
    int r12 = r10 >> 1;
    int r13 = r11 & 0xFF;
    int r14 = r12 | r13;
    int r15 = r14 * 3;
    int r16 = r15 + a;
    int r17 = r16 ^ b;
    int r18 = r17 * c;
    int r19 = r18 - d;
    int r20 = r19 + e;
    
    /* Mix in some memory operations */
    volatile int mem1 = r20;
    int mem2 = mem1;
    volatile int mem3 = mem2 * 2;
    
    return r20 + mem3;
}

/* Function with SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(volatile float f1, volatile float f2, 
                           volatile float f3, volatile float f4) {
    __m128 vec1 = _mm_set_ps(f1, f2, f3, f4);
    __m128 vec2 = _mm_set_ps(f4, f3, f2, f1);
    
    /* Create dependency chain with SSE operations */
    __m128 r1 = _mm_add_ps(vec1, vec2);
    __m128 r2 = _mm_mul_ps(r1, vec1);
    __m128 r3 = _mm_sub_ps(r2, vec2);
    __m128 r4 = _mm_add_ps(r3, r1);
    
    /* Horizontal add pattern */
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(2, 3, 0, 1)));
    r4 = _mm_add_ps(r4, _mm_shuffle_ps(r4, r4, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, r4);
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(volatile int a, volatile int b, volatile int c) {
    int result;
    
    /* Create artificial dependency chain with barriers */
    asm volatile ("addl %1, %0" : "=r"(result) : "r"(a), "0"(0));
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    int temp = b * 7;
    
    /* Another barrier */
    asm volatile ("# barrier" : : : "memory");
    
    asm volatile ("imull %1, %0" : "+r"(temp) : "r"(c));
    
    /* Complex inline asm with multiple constraints */
    asm volatile (
        "movl %1, %%eax\n\t"
        "addl %2, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(temp), "r"(a)
        : "%eax", "memory"
    );
    
    return result;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(volatile int limit, volatile int seed) {
    int total = 0;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable branch */
        if (seed & (1 << (i & 7))) {
            /* Create dependency chain in taken branch */
            int x = i * 3;
            int y = x + seed;
            int z = y ^ (x << 2);
            total += z;
            
            /* Inline asm in taken path */
            asm volatile ("addl $1, %0" : "+r"(total) : : "cc");
        } else {
            /* Different operations in not-taken branch */
            int x = i + seed;
            int y = x * 5;
            int z = y - (seed >> 1);
            total -= z;
            
            /* Another inline asm */
            asm volatile ("subl $1, %0" : "+r"(total) : : "cc");
        }
        
        /* Modify seed to change branch pattern */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return total;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 42;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 123;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 789;
    volatile float fseed1 = (float)(seed1 % 100) / 10.0f;
    volatile float fseed2 = (float)(seed2 % 100) / 10.0f;
    
    int total = 0;
    
    /* Execute multiple iterations to increase chance of context creation */
    for (int iter = 0; iter < 100; ++iter) {
        /* Mix different patterns to trigger various scheduler behaviors */
        
        /* 1. Dense operations to fill instruction queue */
        total += dense_operations(seed1 + iter);
        
        /* 2. Arithmetic helper with dependencies */
        total += helper_arithmetic(seed2, iter, seed3);
        
        /* 3. SSE operations for target-specific scheduling */
        float fresult = sse_operations(fseed1 + iter, fseed2, 
                                      fseed1 - iter, fseed2 * 2);
        total += (int)fresult;
        
        /* 4. Inline assembly with barriers */
        total += asm_barrier_ops(seed1, seed2 + iter, seed3);
        
        /* 5. Branching pattern for speculative scheduling */
        volatile int limit = 5 + (iter % 10);
        total += branching_pattern(limit, seed3 + iter);
        
        /* Modify seeds to vary patterns */
        seed1 = (seed1 * 1664525 + 1013904223) & 0x7FFFFFFF;
        seed2 = (seed2 * 1103515245 + 12345) & 0x7FFFFFFF;
        seed3 = (seed3 * 134775813 + 1) & 0x7FFFFFFF;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
