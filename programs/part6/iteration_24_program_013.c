/* test_sched_context.c - Comprehensive test for Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Helper function with pure computation - encourages speculative scheduling */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (b * c) + (c * a) - (a ^ b ^ c);
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a - c;
    t5 = b + d;
    t6 = t4 * t5;
    t7 = t3 | t6;
    t8 = a << 2;
    t9 = b >> 1;
    t10 = t8 & t9;
    t11 = c + 0x1234;
    t12 = d * 0x5678;
    t13 = t11 ^ t12;
    t14 = t7 + t10;
    t15 = t13 - t14;
    t16 = t15 * 3;
    t17 = t16 / 2;
    t18 = t17 % 100;
    t19 = t18 << 3;
    t20 = t19 >> 1;
    
    /* Create dependencies between them */
    t1 = t20 + t1;
    t2 = t1 * t2;
    t3 = t2 ^ t3;
    t4 = t3 - t4;
    t5 = t4 + t5;
    t6 = t5 * t6;
    t7 = t6 | t7;
    t8 = t7 & t8;
    t9 = t8 >> t9;
    t10 = t9 ^ t10;
    
    return t10;
}

/* Function with memory operations and barriers */
static int memory_barrier_test(volatile int* mem1, volatile int* mem2, int iter) {
    int sum = 0;
    
    for (int i = 0; i < iter; i++) {
        int val1, val2;
        
        /* Memory reads with compiler barrier */
        val1 = *mem1;
        asm volatile("" ::: "memory");
        val2 = *mem2;
        
        /* Computation */
        int tmp = pure_helper(val1, val2, i);
        
        /* Memory write with barrier */
        asm volatile("" ::: "memory");
        *mem1 = tmp;
        
        sum += tmp;
        
        /* Branch with unpredictable outcome */
        if (tmp % 7 == 0) {
            asm volatile("" ::: "memory");
            *mem2 = sum;
        }
    }
    
    return sum;
}

/* Function with SSE/MMX intrinsics - triggers target-specific scheduling */
static __m128i sse_vector_test(int a, int b, int c, int d) {
    __m128i vec1, vec2, vec3, vec4, result;
    
    /* Create vector operations */
    vec1 = _mm_set_epi32(a, b, c, d);
    vec2 = _mm_set_epi32(d, c, b, a);
    
    /* Multiple vector operations */
    vec3 = _mm_add_epi32(vec1, vec2);
    vec4 = _mm_mullo_epi32(vec1, vec2);
    
    /* Mix with shuffles */
    __m128i shuffle1 = _mm_shuffle_epi32(vec3, _MM_SHUFFLE(0, 1, 2, 3));
    __m128i shuffle2 = _mm_shuffle_epi32(vec4, _MM_SHUFFLE(3, 2, 1, 0));
    
    /* More operations */
    result = _mm_add_epi32(shuffle1, shuffle2);
    result = _mm_sub_epi32(result, vec1);
    result = _mm_add_epi32(result, vec2);
    
    return result;
}

/* Function with inline assembly constraints */
static int inline_asm_test(int a, int b, int c, int d) {
    int out1, out2, out3, out4;
    
    /* Chain of dependent asm operations */
    asm volatile (
        "add %0, %1, %2\n\t"
        : "=r"(out1) : "r"(a), "r"(b)
    );
    
    asm volatile (
        "imul %0, %1, %2\n\t"
        : "=r"(out2) : "r"(c), "r"(d)
    );
    
    /* Barrier between dependent operations */
    asm volatile("" ::: "memory");
    
    asm volatile (
        "xor %0, %1, %2\n\t"
        : "=r"(out3) : "r"(out1), "r"(out2)
    );
    
    /* Another barrier */
    asm volatile("" ::: "memory");
    
    asm volatile (
        "sub %0, %1, %2\n\t"
        : "=r"(out4) : "r"(out3), "r"(a)
    );
    
    return out4;
}

/* Complex loop with varying trip counts and scheduling contexts */
static int complex_loop_test(volatile int seed1, volatile int seed2, int iterations) {
    int total = 0;
    
    for (int outer = 0; outer < iterations; outer++) {
        volatile int trip = (seed1 + outer) % 50 + 10;
        
        for (int inner = 0; inner < trip; inner++) {
            /* Mix different types of operations */
            int val1 = pure_helper(seed1, seed2, inner);
            int val2 = dense_arithmetic(seed1, seed2, inner, outer);
            
            /* SSE operations */
            __m128i vec_result = sse_vector_test(val1, val2, inner, outer);
            int vec_vals[4];
            _mm_storeu_si128((__m128i*)vec_vals, vec_result);
            
            /* Inline assembly */
            int asm_result = inline_asm_test(vec_vals[0], vec_vals[1], 
                                            vec_vals[2], vec_vals[3]);
            
            /* Memory operations with barriers */
            volatile int mem1 = asm_result;
            volatile int mem2 = val1;
            int mem_result = memory_barrier_test(&mem1, &mem2, 5);
            
            total += mem_result + asm_result + val2;
            
            /* Unpredictable branch */
            if ((total ^ (inner * 0x12345678)) % 13 == 0) {
                /* Force scheduler to consider alternative paths */
                asm volatile("" ::: "memory");
                total += seed2;
            }
        }
        
        /* Occasionally reset state */
        if (outer % 7 == 0) {
            asm volatile("" ::: "memory");
            seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        }
    }
    
    return total;
}

/* Main test driver */
int main(int argc, char** argv) {
    /* Use argv for volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Multiple iterations to increase chance of context creation/freeing */
    for (int iter = 0; iter < 100; iter++) {
        /* Call different test functions to exercise various scheduler behaviors */
        
        /* 1. Test with pure computations */
        int pure_result = 0;
        for (int i = 0; i < 20; i++) {
            pure_result += pure_helper(seed1 + i, seed2 + iter, seed3);
        }
        total += pure_result;
        
        /* 2. Test with dense arithmetic */
        int dense_result = dense_arithmetic(seed1, seed2, seed3, iter);
        total += dense_result;
        
        /* 3. Test with SSE operations */
        __m128i sse_result = sse_vector_test(seed1, seed2, seed3, iter);
        int sse_vals[4];
        _mm_storeu_si128((__m128i*)sse_vals, sse_result);
        total += sse_vals[0] + sse_vals[1] + sse_vals[2] + sse_vals[3];
        
        /* 4. Test with inline assembly */
        int asm_result = inline_asm_test(seed1, seed2, seed3, iter);
        total += asm_result;
        
        /* 5. Test with memory barriers */
        volatile int mem1 = seed1 + iter;
        volatile int mem2 = seed2 - iter;
        int mem_result = memory_barrier_test(&mem1, &mem2, 10);
        total += mem_result;
        
        /* 6. Complex loop test (most likely to trigger context saves) */
        int complex_result = complex_loop_test(seed1 + iter, seed2 - iter, 5);
        total += complex_result;
        
        /* Modify seeds to create varying behavior */
        seed1 = (seed1 * 1664525 + 1013904223) & 0x7fffffff;
        seed2 = (seed2 * 1103515245 + 12345) & 0x7fffffff;
        
        /* Insert barrier occasionally */
        if (iter % 19 == 0) {
            asm volatile("" ::: "memory");
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return total & 0xff;  /* Return non-zero to indicate execution */
}
