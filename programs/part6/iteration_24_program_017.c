#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling complexity */

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, int e) {
    volatile int v1 = a;  /* Prevent optimization */
    volatile int v2 = b;
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    
    /* Create many independent arithmetic operations */
    t1 = v1 + v2;
    t2 = v1 * v2;
    t3 = v1 ^ v2;
    t4 = v1 - v2;
    t5 = v1 & v2;
    t6 = v1 | v2;
    t7 = t1 * t2;
    t8 = t3 ^ t4;
    t9 = t5 & t6;
    t10 = t7 + t8;
    
    /* More operations with dependencies */
    for (int i = 0; i < 3; i++) {
        t1 = t1 + t2;
        t2 = t2 * t3;
        t3 = t3 ^ t4;
        t4 = t4 - t5;
        t5 = t5 & t6;
    }
    
    /* Memory operations to introduce load/store dependencies */
    int *ptr1 = &t1;
    int *ptr2 = &t2;
    *ptr1 = t10;
    int tmp = *ptr2;
    *ptr2 = tmp + t9;
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(int a, int b, int c) {
    int result = 0;
    volatile int barrier = 0;
    
    /* Initial computation */
    int x = a * b + c;
    
    /* Assembly with memory clobber - acts as scheduling barrier */
    asm volatile ("# Start barrier\n\t"
                  "addl %1, %0\n\t"
                  : "+r" (x)
                  : "r" (c)
                  : "memory");
    
    /* More computations that scheduler might try to move across barrier */
    int y = x ^ b;
    
    /* Another barrier */
    asm volatile ("# Middle barrier\n\t"
                  "imull %1, %0\n\t"
                  : "+r" (y)
                  : "r" (a)
                  : "memory");
    
    /* Complex dependency chain */
    for (int i = 0; i < (barrier ? 5 : 3); i++) {
        /* Volatile read to prevent reordering */
        barrier = *(volatile int *)&barrier;
        y = y + i + barrier;
        
        /* Inline assembly with dependencies */
        asm volatile ("addl %1, %0\n\t"
                      : "+r" (y)
                      : "r" (a)
                      : "cc");
    }
    
    result = x + y;
    
    /* Final barrier */
    asm volatile ("# End barrier\n\t"
                  : : : "memory");
    
    return result;
}

/* Function using SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(float a, float b, float c, float d) {
    __m128 vec1 = _mm_set_ps(a, b, c, d);
    __m128 vec2 = _mm_set_ps(b, c, d, a);
    __m128 vec3 = _mm_set_ps(c, d, a, b);
    
    /* Mix of SSE operations */
    __m128 result = _mm_add_ps(vec1, vec2);
    result = _mm_mul_ps(result, vec3);
    result = _mm_sub_ps(result, vec1);
    
    /* Horizontal add pattern - creates specific dependencies */
    result = _mm_add_ps(result, _mm_shuffle_ps(result, result, _MM_SHUFFLE(2, 3, 0, 1)));
    result = _mm_add_ps(result, _mm_shuffle_ps(result, result, _MM_SHUFFLE(1, 0, 3, 2)));
    
    /* Extract result */
    float res[4];
    _mm_store_ps(res, result);
    
    return res[0] + res[1] + res[2] + res[3];
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_ops(int a, int b, volatile int *control) {
    int result = 0;
    
    /* Unpredictable branch */
    if (*control & 0x1) {
        /* First branch path with computations */
        for (int i = 0; i < (*control & 0x3); i++) {
            result += a * i + b;
            
            /* Inline assembly in branch */
            asm volatile ("imull %%ecx, %%eax\n\t"
                          : "+a" (result)
                          : "c" (b)
                          : "cc");
        }
    } else {
        /* Alternative path */
        result = a - b;
        for (int j = 0; j < 4; j++) {
            result ^= (a << j) | (b >> j);
        }
    }
    
    /* Common code after branch with more operations */
    int temp = result;
    for (int k = 0; k < 8; k++) {
        temp = (temp * 1103515245 + 12345) & 0x7fffffff;
        result ^= temp;
    }
    
    return result;
}

/* Function that mixes all patterns in a loop */
static int mixed_scheduling_patterns(int seed, int iterations) {
    volatile int control = seed;  /* Prevent constant propagation */
    int total = 0;
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Change control value to affect branching */
        control = (control * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Call different patterns to create varied scheduling contexts */
        total += dense_arithmetic(control, control + 1, control + 2, 
                                 control + 3, control + 4);
        
        total += asm_barrier_ops(control & 0xFF, (control >> 8) & 0xFF, 
                                (control >> 16) & 0xFF);
        
        /* SSE operations with float conversion */
        float f1 = (control & 0xFF) / 256.0f;
        float f2 = ((control >> 8) & 0xFF) / 256.0f;
        float f3 = ((control >> 16) & 0xFF) / 256.0f;
        float f4 = (control & 0x7F) / 128.0f;
        total += (int)(sse_operations(f1, f2, f3, f4) * 100);
        
        total += branching_ops(control, control ^ 0x55555555, &control);
        
        /* Additional inline assembly block with resource constraints */
        int tmp = total;
        asm volatile ("# Complex dependency chain\n\t"
                      "movl %1, %%eax\n\t"
                      "addl %%eax, %%eax\n\t"
                      "imull %2, %%eax\n\t"
                      "addl %%eax, %0\n\t"
                      : "+r" (total)
                      : "r" (control), "r" (iter)
                      : "%eax", "cc");
    }
    
    return total;
}

int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int vseed1 = seed1;  /* Volatile to prevent optimization */
    volatile int vseed2 = seed2;
    
    int total_result = 0;
    
    /* Multiple iterations to increase chance of scheduling context creation */
    for (int outer = 0; outer < 100; outer++) {
        /* Vary the iteration count to create different scheduling scenarios */
        int iterations = (vseed1 + outer) % 10 + 5;
        
        /* Call the mixed patterns function */
        int result = mixed_scheduling_patterns(vseed1 ^ outer, iterations);
        
        /* Additional complex computation with inline assembly */
        int temp = result;
        asm volatile ("# Final computation block\n\t"
                      "movl %1, %%eax\n\t"
                      "rorl $13, %%eax\n\t"
                      "xorl %2, %%eax\n\t"
                      "addl %%eax, %0\n\t"
                      : "+r" (total_result)
                      : "r" (temp), "r" (vseed2)
                      : "%eax", "cc");
        
        /* Update volatile seeds */
        vseed1 = (vseed1 * 1103515245 + 12345) & 0x7fffffff;
        vseed2 = (vseed2 * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return total_result & 0xFF;
}
