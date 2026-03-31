/* Test program to exercise haifa-sched.cc free_sched_context logic */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
volatile int seed1, seed2, seed3;

/* Helper with arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c) {
    int t1 = a + b;
    int t2 = b * c;
    int t3 = t1 ^ t2;
    int t4 = t2 - a;
    int t5 = t3 * t4;
    int t6 = t5 + c;
    int t7 = t6 ^ a;
    int t8 = t7 - b;
    int t9 = t8 * t3;
    int t10 = t9 ^ t4;
    int t11 = t10 + t5;
    int t12 = t11 - t6;
    int t13 = t12 * t7;
    int t14 = t13 ^ t8;
    int t15 = t14 + t9;
    int t16 = t15 - t10;
    int t17 = t16 * t11;
    int t18 = t17 ^ t12;
    int t19 = t18 + t13;
    int t20 = t19 - t14;
    
    /* Memory operations to introduce dependencies */
    volatile int mem1 = t15;
    volatile int mem2 = t16;
    int t21 = mem1 + mem2;
    
    int t22 = t20 * t21;
    int t23 = t22 ^ t17;
    int t24 = t23 + t18;
    int t25 = t24 - t19;
    int t26 = t25 * t20;
    int t27 = t26 ^ t21;
    int t28 = t27 + t22;
    int t29 = t28 - t23;
    int t30 = t29 * t24;
    
    return t30;
}

/* Helper with inline assembly barriers to force state restoration */
static int asm_barrier_sequence(int a, int b) {
    int r1, r2, r3;
    
    /* Initial computation */
    r1 = a * b + 1;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : "=r"(r2) : "0"(r1) : "memory");
    
    /* Dependent computation */
    r2 = r2 * 3 - 7;
    
    /* Another barrier */
    asm volatile ("# barrier" : : : "memory");
    
    /* More computation */
    r3 = r2 ^ a;
    
    /* Complex inline asm with dependencies */
    asm volatile (
        "addl %1, %0\n\t"
        "imull $0x1234, %0, %0"
        : "+r"(r3)
        : "r"(b)
        : "cc"
    );
    
    return r3;
}

/* Helper with vector intrinsics to trigger target-specific scheduling */
static __m128i vector_operations(__m128i a, __m128i b) {
    __m128i r1, r2, r3, r4;
    
    /* Mix of vector operations */
    r1 = _mm_add_epi32(a, b);
    r2 = _mm_mullo_epi16(a, b);
    r3 = _mm_slli_epi32(r1, 3);
    r4 = _mm_xor_si128(r2, r3);
    
    /* More operations to create scheduling complexity */
    r1 = _mm_add_epi32(r4, a);
    r2 = _mm_sub_epi32(r1, b);
    r3 = _mm_madd_epi16(r2, r4);
    
    /* Shuffle to introduce dependencies */
    r4 = _mm_shuffle_epi32(r3, _MM_SHUFFLE(1, 0, 3, 2));
    
    return _mm_add_epi32(r3, r4);
}

/* Helper with branching for speculative scheduling */
static int branching_sequence(int a, int b, int iter) {
    int result = 0;
    
    /* Loop with volatile iteration count */
    for (int i = 0; i < iter; ++i) {
        /* Unpredictable branch */
        if (a & (1 << (i & 7))) {
            /* Inline asm with resource constraints */
            int temp;
            asm volatile (
                "movl %1, %0\n\t"
                "addl $0x7, %0\n\t"
                "imull %2, %0"
                : "=r"(temp)
                : "r"(a), "r"(i)
                : "cc"
            );
            result += temp;
        } else {
            /* Different computation path */
            int temp;
            asm volatile (
                "movl %1, %0\n\t"
                "subl $0x3, %0\n\t"
                "xorl %2, %0"
                : "=r"(temp)
                : "r"(b), "r"(i)
                : "cc"
            );
            result ^= temp;
        }
        
        /* Small pure function call as scheduling barrier */
        {
            int x = (result * 1103515245 + 12345) & 0x7fffffff;
            result = x % 1000;
        }
    }
    
    return result;
}

/* Complex loop with mixed operations */
static int mixed_loop_sequence(int a, int b, int c) {
    int sum = 0;
    volatile int iter = seed3 & 0x3F; /* Prevent loop unrolling */
    
    for (int i = 0; i < iter + 1; ++i) {
        /* Mix different types of operations */
        int arith = dense_arithmetic(a + i, b, c);
        int barrier = asm_barrier_sequence(arith, b);
        
        /* Vector operations every few iterations */
        if (i % 4 == 0) {
            __m128i v1 = _mm_set1_epi32(a + i);
            __m128i v2 = _mm_set1_epi32(b);
            __m128i vr = vector_operations(v1, v2);
            int arr[4];
            _mm_storeu_si128((__m128i*)arr, vr);
            barrier += arr[0] + arr[1];
        }
        
        /* Branching sequence */
        if (barrier & 1) {
            sum += branching_sequence(barrier, c, 3);
        } else {
            sum -= barrier;
        }
        
        /* Memory clobber to force scheduling boundaries */
        asm volatile ("" : : : "memory");
    }
    
    return sum;
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant folding */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the inputs to create different scheduling patterns */
        int a = seed1 + outer;
        int b = seed2 ^ outer;
        int c = seed3 * (outer + 1);
        
        /* Call different helpers to exercise various scheduler paths */
        total += dense_arithmetic(a, b, c);
        total += asm_barrier_sequence(b, c);
        
        /* Vector operations */
        __m128i v1 = _mm_set1_epi32(a);
        __m128i v2 = _mm_set1_epi32(b);
        __m128i vr = vector_operations(v1, v2);
        int arr[4];
        _mm_storeu_si128((__m128i*)arr, vr);
        total += arr[0] + arr[2];
        
        total += branching_sequence(c, a, 5);
        total += mixed_loop_sequence(a, b, c);
        
        /* Occasionally add memory pressure */
        if (outer % 23 == 0) {
            volatile int* ptr = (int*)malloc(16 * sizeof(int));
            if (ptr) {
                for (int i = 0; i < 16; ++i) {
                    ptr[i] = total + i;
                }
                for (int i = 0; i < 16; ++i) {
                    total ^= ptr[i];
                }
                free((void*)ptr);
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
