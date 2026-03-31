/* test_sched_context.c - Trigger Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create different scheduling patterns */

/* Function 1: Creates a dense sequence of arithmetic operations */
static int dense_arithmetic_sequence(volatile int seed) {
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x55AA;
    int d = seed >> 3;
    int e = seed << 2;
    
    /* Create many independent operations to fill instruction queue */
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t2 - e;
    int t5 = t3 * t4;
    int t6 = t1 & t4;
    int t7 = t5 | t6;
    int t8 = t7 << 1;
    int t9 = t8 >> 2;
    int t10 = t9 * a;
    int t11 = t10 + b;
    int t12 = t11 - c;
    int t13 = t12 * d;
    int t14 = t13 ^ e;
    int t15 = t14 + t1;
    int t16 = t15 * t2;
    int t17 = t16 ^ t3;
    int t18 = t17 | t4;
    int t19 = t18 & t5;
    int t20 = t19 + t6;
    
    /* Mix in memory operations */
    volatile int mem_var = seed;
    int* ptr = (int*)&mem_var;
    int tmp1 = *ptr;
    *ptr = t20;
    int tmp2 = *ptr;
    
    return tmp1 + tmp2 + t7 + t8 + t9 + t10;
}

/* Function 2: Uses vector intrinsics to trigger target-specific scheduling */
static __m128 vector_operations(volatile int seed) {
    /* Create vector data */
    float f1 = (float)(seed & 0xFF);
    float f2 = (float)((seed >> 8) & 0xFF);
    float f3 = (float)((seed >> 16) & 0xFF);
    float f4 = (float)((seed >> 24) & 0xFF);
    
    __m128 v1 = _mm_set_ps(f1, f2, f3, f4);
    __m128 v2 = _mm_set1_ps(2.0f);
    __m128 v3 = _mm_set1_ps(3.0f);
    
    /* Chain of vector operations */
    __m128 r1 = _mm_add_ps(v1, v2);
    __m128 r2 = _mm_mul_ps(r1, v3);
    __m128 r3 = _mm_sub_ps(r2, v1);
    __m128 r4 = _mm_add_ps(r3, r1);
    __m128 r5 = _mm_mul_ps(r4, v2);
    
    /* SSE2 integer operations */
    __m128i vi1 = _mm_set1_epi32(seed);
    __m128i vi2 = _mm_set1_epi32(0x12345678);
    __m128i vi3 = _mm_add_epi32(vi1, vi2);
    __m128i vi4 = _mm_sub_epi32(vi3, vi1);
    
    /* Use inline asm to create scheduling barriers */
    asm volatile ("" : : : "memory");
    
    /* More vector operations after barrier */
    __m128 r6 = _mm_add_ps(r5, _mm_set1_ps(1.0f));
    __m128 r7 = _mm_mul_ps(r6, r5);
    
    /* Another memory barrier */
    asm volatile ("" : : : "memory");
    
    return _mm_add_ps(r7, r4);
}

/* Function 3: Creates complex control flow with inline assembly */
static int control_flow_with_asm(volatile int seed, volatile int iter) {
    int result = 0;
    
    /* Loop with volatile iteration count */
    for (int i = 0; i < iter; ++i) {
        int a = seed + i;
        int b = seed * i;
        
        /* Inline assembly with data dependencies */
        int out1, out2;
        asm volatile (
            "add %0, %1, %2\n\t"
            "mul %3, %1, %2"
            : "=r"(out1), "=r"(out2)
            : "r"(a), "r"(b)
            : "cc"
        );
        
        /* Branch with unpredictable outcome */
        if (a & 1) {
            /* More inline assembly in taken branch */
            int out3;
            asm volatile (
                "xor %0, %1, %2"
                : "=r"(out3)
                : "r"(out1), "r"(out2)
            );
            result += out3;
        } else {
            /* Different operations in not-taken branch */
            int out4;
            asm volatile (
                "and %0, %1, %2"
                : "=r"(out4)
                : "r"(out1), "r"(i)
            );
            result += out4;
        }
        
        /* Memory clobber to create scheduling barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function 4: Pure function called with volatile arguments */
static int pure_helper(int a, int b) {
    /* Simple pure computation */
    return a * b + (a ^ b) - (a & b);
}

static int calls_with_volatile(volatile int seed) {
    int total = 0;
    
    /* Call pure function multiple times with volatile args */
    for (int i = 0; i < 10; ++i) {
        volatile int arg1 = seed + i;
        volatile int arg2 = seed - i;
        
        /* The call acts as a scheduling barrier */
        total += pure_helper(arg1, arg2);
        
        /* Insert scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* More operations after call */
        int tmp = arg1 * 3 + arg2 / 2;
        total ^= tmp;
    }
    
    return total;
}

/* Function 5: Mixed integer and memory operations */
static int mixed_operations(volatile int seed) {
    int array[16];
    int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 16; ++i) {
        array[i] = seed + i;
    }
    
    /* Chain of dependent operations */
    int t1 = array[0] + array[1];
    int t2 = array[2] * array[3];
    int t3 = t1 ^ t2;
    int t4 = array[4] - array[5];
    int t5 = t3 & t4;
    int t6 = array[6] | array[7];
    int t7 = t5 * t6;
    int t8 = array[8] ^ array[9];
    int t9 = t7 + t8;
    int t10 = array[10] & array[11];
    int t11 = t9 - t10;
    int t12 = array[12] | array[13];
    int t13 = t11 ^ t12;
    int t14 = array[14] * array[15];
    int t15 = t13 + t14;
    
    /* Memory operations mixed in */
    volatile int mem1 = t15;
    int* ptr1 = (int*)&mem1;
    int tmp1 = *ptr1;
    
    *ptr1 = tmp1 + seed;
    int tmp2 = *ptr1;
    
    result = tmp1 + tmp2 + t15;
    
    /* Final memory barrier */
    asm volatile ("" : : : "memory");
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv to create volatile seeds to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int iter_count = argc > 4 ? atoi(argv[4]) : 100;
    
    long long total = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < iter_count; ++outer) {
        volatile int loop_seed = seed1 + outer;
        
        /* Call different helper functions to create varied scheduling patterns */
        
        /* 1. Dense arithmetic sequence */
        total += dense_arithmetic_sequence(loop_seed);
        
        /* 2. Vector operations (triggers target-specific scheduling) */
        __m128 vec_result = vector_operations(loop_seed);
        float vec_sum;
        _mm_store_ss(&vec_sum, vec_result);
        total += (int)vec_sum;
        
        /* 3. Control flow with inline assembly */
        volatile int iter = (loop_seed % 10) + 5;
        total += control_flow_with_asm(loop_seed, iter);
        
        /* 4. Function calls with volatile arguments */
        total += calls_with_volatile(loop_seed ^ seed2);
        
        /* 5. Mixed operations */
        total += mixed_operations(loop_seed + seed3);
        
        /* Occasionally add a scheduling barrier in outer loop */
        if (outer % 7 == 0) {
            asm volatile ("" : : : "memory");
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %lld\n", total);
    
    return (int)(total % 256);
}
