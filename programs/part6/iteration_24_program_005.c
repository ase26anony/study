/* test_sched_context.c - Comprehensive test for Haifa scheduler context cleanup */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* For SSE intrinsics */
#include <emmintrin.h>  /* For SSE2 intrinsics */

/* Helper function to create scheduling complexity */
static int helper_compute(int a, int b, int c) {
    /* Complex sequence of dependent operations */
    int t1 = a * b + c;
    int t2 = (a ^ b) | c;
    int t3 = t1 * t2 - a;
    int t4 = (t3 << 3) | (t2 >> 2);
    int t5 = t4 * t1 + t2;
    return t5 % 1023;
}

/* Function with dense arithmetic operations to fill instruction queue */
static int dense_arithmetic(volatile int seed) {
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed - 100;
    int e = seed | 0xFF00;
    
    /* Large sequence of independent operations */
    int r1 = a + b;
    int r2 = c * d;
    int r3 = a ^ e;
    int r4 = b | c;
    int r5 = d - a;
    int r6 = e * c;
    int r7 = a & d;
    int r8 = b ^ e;
    int r9 = c + r1;
    int r10 = d * r2;
    int r11 = e | r3;
    int r12 = r1 ^ r4;
    int r13 = r2 + r5;
    int r14 = r3 * r6;
    int r15 = r4 - r7;
    int r16 = r5 | r8;
    int r17 = r6 ^ r9;
    int r18 = r7 + r10;
    int r19 = r8 * r11;
    int r20 = r9 | r12;
    
    /* Combine results */
    return r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
           r11 + r12 + r13 + r14 + r15 + r16 + r17 + r18 + r19 + r20;
}

/* Function with SSE intrinsics to trigger target-specific scheduling */
static float sse_operations(volatile float f1, volatile float f2, 
                           volatile float f3, volatile float f4) {
    __m128 v1 = _mm_set_ps(f1, f2, f3, f4);
    __m128 v2 = _mm_set_ps(f4, f3, f2, f1);
    
    /* Sequence of vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(v1, v2);
    __m128 r3 = _mm_sub_ps(r1, r2);
    __m128 r4 = _mm_add_ps(r3, v1);
    __m128 r5 = _mm_mul_ps(r4, v2);
    __m128 r6 = _mm_sub_ps(r5, r3);
    
    /* Horizontal add */
    r6 = _mm_add_ps(r6, _mm_shuffle_ps(r6, r6, _MM_SHUFFLE(2, 3, 0, 1)));
    r6 = _mm_add_ps(r6, _mm_shuffle_ps(r6, r6, _MM_SHUFFLE(1, 0, 3, 2)));
    
    float result;
    _mm_store_ss(&result, r6);
    return result;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_ops(volatile int x, volatile int y) {
    int a = x, b = y;
    int result = 0;
    
    /* Sequence with assembly barriers */
    a = a * 2 + 1;
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile("" : : : "memory");
    
    b = b ^ 0x1234;
    
    /* Another barrier */
    asm volatile("" : : : "memory");
    
    result = a + b;
    
    /* Complex inline assembly with dependencies */
    asm volatile (
        "addl %1, %0\n\t"
        "xorl %2, %0\n\t"
        "imull $0x55, %0"
        : "+r" (result)
        : "r" (a), "r" (b)
        : "cc"
    );
    
    asm volatile("" : : : "memory");
    
    return result;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(volatile int limit) {
    int sum = 0;
    
    /* Loop with volatile condition to prevent optimization */
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable branch */
        if (i & 1) {
            sum += helper_compute(i, sum, limit);
        } else {
            sum -= helper_compute(sum, i, limit);
        }
        
        /* Additional computation with memory operations */
        volatile int* ptr = &sum;
        int temp = *ptr;
        temp = temp * 3 + i;
        *ptr = temp;
        
        /* Small inline assembly to create scheduling constraints */
        asm volatile (
            "movl %1, %%eax\n\t"
            "leal (%%eax, %%eax, 2), %%eax\n\t"
            "movl %%eax, %0"
            : "=r" (temp)
            : "r" (temp)
            : "%eax"
        );
        
        sum = temp;
    }
    
    return sum;
}

/* Function mixing all patterns in a complex loop */
static int mixed_complex_loop(volatile int iterations) {
    int total = 0;
    float ftotal = 0.0f;
    
    for (int i = 0; i < iterations; ++i) {
        /* Alternate between different patterns */
        switch (i & 3) {
            case 0:
                total += dense_arithmetic(i + total);
                break;
            case 1:
                ftotal += sse_operations(i * 0.1f, total * 0.01f, 
                                        (i + total) * 0.001f, 1.0f);
                total += (int)ftotal;
                break;
            case 2:
                total ^= asm_barrier_ops(i, total);
                break;
            case 3:
                total = helper_compute(total, i, iterations);
                break;
        }
        
        /* Insert memory operation to create dependencies */
        volatile int mem_var = total;
        total = mem_var + i;
        
        /* Occasionally call branching function */
        if ((i & 7) == 0) {
            total += branching_pattern((i & 15) + 1);
        }
    }
    
    return total + (int)ftotal;
}

int main(int argc, char** argv) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 13579;
    volatile float fseed = (float)(seed1 % 100) * 0.01f;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of context creation/deletion */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the patterns across outer iterations */
        volatile int iter_count = (outer % 20) + 5;
        
        switch (outer % 4) {
            case 0:
                total += dense_arithmetic(seed1 + outer);
                break;
            case 1:
                total += (int)sse_operations(fseed, fseed * 2.0f, 
                                           fseed * 3.0f, fseed * 4.0f);
                break;
            case 2:
                total ^= asm_barrier_ops(seed2 + outer, total);
                break;
            case 3:
                total = helper_compute(total, seed3 + outer, iter_count);
                break;
        }
        
        /* Every 10 iterations, run the complex mixed loop */
        if (outer % 10 == 0) {
            total += mixed_complex_loop(iter_count);
        }
        
        /* Branch with volatile condition */
        volatile int branch_cond = seed1 + outer;
        if (branch_cond & 1) {
            total += branching_pattern((outer & 7) + 3);
        }
        
        /* Memory operations to create scheduling constraints */
        volatile int* volatile ptr = &total;
        int temp = *ptr;
        temp = temp * 1103515245 + 12345;
        *ptr = temp & 0x7fffffff;
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
