/* test_sched_context.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Volatile helper to prevent constant propagation */
static volatile int volatile_seed;

/* Helper function with scheduling barriers */
static int helper_with_barrier(int a, int b) {
    int result;
    /* Create artificial scheduling barrier */
    asm volatile ("" : : : "memory");
    result = a * b + 1;
    asm volatile ("" : : : "memory");
    return result;
}

/* Function with dense arithmetic sequence to fill instruction queue */
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
    t8 = a * c;
    t9 = b * d;
    t10 = t8 - t9;
    t11 = t7 ^ t10;
    t12 = a + d;
    t13 = b + c;
    t14 = t12 * t13;
    t15 = t11 & t14;
    t16 = t1 + t2 + t3;
    t17 = t4 + t5 + t6;
    t18 = t7 + t8 + t9;
    t19 = t10 + t11 + t12;
    t20 = t13 + t14 + t15;
    
    /* Mix with memory operations */
    volatile int *ptr = &volatile_seed;
    int mem_val = *ptr;
    t1 += mem_val;
    t20 -= mem_val;
    
    /* More operations to ensure large ready list */
    t1 = t1 * t2 - t3;
    t2 = t4 / (t5 | 1);
    t3 = t6 ^ t7 ^ t8;
    t4 = t9 + t10 - t11;
    t5 = t12 * t13 / 2;
    t6 = t14 | t15 | t16;
    t7 = t17 & t18 & t19;
    t8 = t20 ^ t1 ^ t2;
    
    return t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
}

/* Function with SSE/MMX intrinsics to trigger target-specific scheduling */
static __m128 vector_operations(__m128 a, __m128 b, __m128 c) {
    __m128 r1, r2, r3, r4, r5, r6;
    
    /* Mix of vector operations */
    r1 = _mm_add_ps(a, b);
    r2 = _mm_mul_ps(b, c);
    r3 = _mm_sub_ps(a, c);
    r4 = _mm_add_ps(r1, r2);
    r5 = _mm_mul_ps(r3, r4);
    r6 = _mm_add_ps(r5, a);
    
    /* Use inline assembly with specific constraints */
    __m128 result;
    asm volatile (
        "addps %1, %2\n\t"
        "mulps %2, %0\n\t"
        : "=x"(result)
        : "x"(r6), "x"(b)
        : 
    );
    
    return result;
}

/* Function with complex control flow and inline assembly barriers */
static int control_flow_with_barriers(int a, int b, int iter) {
    int result = 0;
    volatile int condition;
    
    for (int i = 0; i < iter; ++i) {
        /* Unpredictable branch */
        condition = volatile_seed + i;
        
        if (condition & 1) {
            /* Branch with scheduling barrier */
            asm volatile ("" : : : "memory");
            result += a * i;
            asm volatile ("" : : : "memory");
        } else {
            /* Alternative path with different operations */
            result -= b / (i + 1);
        }
        
        /* More operations in loop body */
        int t1 = a + i;
        int t2 = b - i;
        result ^= t1 * t2;
        
        /* Another memory barrier */
        asm volatile ("" : : : "memory");
    }
    
    return result;
}

/* Function with mixed integer/float operations */
static float mixed_operations(int a, float b, double c) {
    float f1 = (float)a + b;
    double d1 = (double)f1 * c;
    int i1 = (int)d1;
    float f2 = b * (float)i1;
    
    /* Inline assembly with mixed constraints */
    float result;
    asm volatile (
        "cvtsi2ss %1, %%xmm0\n\t"
        "addss %2, %%xmm0\n\t"
        "mulss %3, %%xmm0\n\t"
        "movss %%xmm0, %0\n\t"
        : "=m"(result)
        : "r"(a), "m"(b), "m"(f2)
        : "xmm0", "memory"
    );
    
    return result;
}

/* Main function that orchestrates all patterns */
int main(int argc, char *argv[]) {
    /* Initialize volatile seeds from argv to prevent constant propagation */
    volatile_seed = (argc > 1) ? atoi(argv[1]) : 12345;
    volatile int iter_seed = (argc > 2) ? atoi(argv[2]) : 100;
    volatile int loop_seed = (argc > 3) ? atoi(argv[3]) : 50;
    
    int total_result = 0;
    float float_total = 0.0f;
    
    /* Create vector data for SSE operations */
    __m128 vec_a = _mm_set1_ps(1.5f);
    __m128 vec_b = _mm_set1_ps(2.5f);
    __m128 vec_c = _mm_set1_ps(3.5f);
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < loop_seed; ++outer) {
        /* Vary the iteration count to create different scheduling contexts */
        int dynamic_iter = (volatile_seed + outer) % 100 + 10;
        
        /* Pattern 1: Dense arithmetic to fill instruction queue */
        total_result += dense_arithmetic(
            volatile_seed + outer,
            volatile_seed - outer,
            outer * 2,
            outer * 3
        );
        
        /* Pattern 2: Control flow with barriers */
        total_result += control_flow_with_barriers(
            volatile_seed,
            outer,
            dynamic_iter
        );
        
        /* Pattern 3: Vector operations for target-specific scheduling */
        __m128 vec_result = vector_operations(vec_a, vec_b, vec_c);
        float vec_float[4];
        _mm_store_ps(vec_float, vec_result);
        total_result += (int)vec_float[0];
        
        /* Pattern 4: Mixed operations */
        float_total += mixed_operations(
            volatile_seed + outer,
            (float)outer * 0.5f,
            (double)outer * 0.25
        );
        
        /* Pattern 5: Helper with scheduling barriers */
        total_result += helper_with_barrier(volatile_seed, outer);
        
        /* Occasionally create deeper nesting */
        if (outer % 7 == 0) {
            for (int inner = 0; inner < 5; ++inner) {
                total_result += dense_arithmetic(
                    inner,
                    outer,
                    volatile_seed,
                    inner * outer
                );
            }
        }
        
        /* Modify volatile seed to change scheduling conditions */
        volatile_seed += outer;
    }
    
    /* Make results observable */
    printf("Total result: %d\n", total_result);
    printf("Float total: %f\n", float_total);
    
    return total_result & 0xFF;  /* Return non-zero to indicate execution */
}
