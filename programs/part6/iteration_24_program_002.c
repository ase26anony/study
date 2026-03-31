/* test_sched_context.c - Trigger Haifa scheduler context allocation/freeing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
static volatile int seed1, seed2, seed3;

/* Helper with pure computation to encourage speculative scheduling */
static int pure_helper(int a, int b, int c) {
    return (a * b) ^ (b * c) ^ (c * a);
}

/* Function with dense independent arithmetic operations */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to fill scheduler structures */
    t1 = a + b;
    t2 = c * d;
    t3 = a ^ b ^ c;
    t4 = t1 * t2;
    t5 = t3 << 2;
    t6 = t4 - t5;
    t7 = b * c * d;
    t8 = a | b | c;
    t9 = t6 ^ t7;
    t10 = t8 & 0xFFFF;
    
    t11 = t9 * t10;
    t12 = t11 >> 4;
    t13 = t12 + a;
    t14 = t13 - b;
    t15 = t14 * c;
    t16 = t15 / (d ? d : 1);
    t17 = t16 ^ t11;
    t18 = t17 << 1;
    t19 = t18 | t12;
    t20 = t19 & 0xFF;
    
    return t20 + t10 + t5;
}

/* Function with vector operations to trigger target-specific scheduling */
static __m128 vector_operations(__m128 a, __m128 b, __m128 c) {
    __m128 t1, t2, t3, t4, t5, t6, t7, t8;
    
    /* Multiple vector operations to engage vector scheduler */
    t1 = _mm_add_ps(a, b);
    t2 = _mm_mul_ps(b, c);
    t3 = _mm_sub_ps(t1, t2);
    t4 = _mm_add_ps(t3, a);
    t5 = _mm_mul_ps(t4, b);
    t6 = _mm_sub_ps(t5, c);
    t7 = _mm_add_ps(t6, t1);
    t8 = _mm_mul_ps(t7, t3);
    
    /* Mix with some integer vector ops if available */
    __m128i i1 = _mm_set1_epi32(seed1);
    __m128i i2 = _mm_set1_epi32(seed2);
    __m128i i3 = _mm_add_epi32(i1, i2);
    (void)i3; /* Use result to prevent elimination */
    
    return t8;
}

/* Function with inline assembly barriers to cause state restoration */
static int asm_barrier_ops(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation cluster */
    asm volatile ("# BEGIN BARRIER 1" ::: "memory");
    res1 = a * b + c;
    asm volatile ("# MIDDLE BARRIER 1" ::: "memory");
    res1 = res1 ^ (a << 3);
    
    /* Second computation cluster with dependency */
    asm volatile ("# BEGIN BARRIER 2" ::: "memory");
    res2 = b * c + a;
    asm volatile ("# MIDDLE BARRIER 2" ::: "memory");
    res2 = res2 ^ (b << 2);
    
    /* Third cluster mixing results */
    asm volatile ("# BEGIN BARRIER 3" ::: "memory");
    res3 = res1 * res2;
    asm volatile ("# MIDDLE BARRIER 3" ::: "memory");
    res3 = res3 + c;
    
    return res3;
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_pattern(int a, int b, int limit) {
    int total = 0;
    volatile int cond = seed3; /* Force memory read */
    
    for (int i = 0; i < limit; i++) {
        /* Unpredictable branch */
        if (cond & (1 << (i & 7))) {
            /* Branch taken path with computation */
            int t = pure_helper(a + i, b - i, i);
            total += t * 2;
        } else {
            /* Branch not taken path with different computation */
            int t = pure_helper(a - i, b + i, i);
            total += t / 2;
        }
        
        /* Modify condition unpredictably */
        cond ^= (i * 16777619);
    }
    
    return total;
}

/* Complex loop with mixed operations */
static int mixed_loop_operations(int iterations) {
    int sum = 0;
    volatile int viter = iterations; /* Prevent loop unrolling elimination */
    
    for (int i = 0; i < viter; ++i) {
        /* Mix different types of operations in the loop body */
        int base = i + seed1;
        
        /* Dense arithmetic section */
        int arith = dense_arithmetic(base, seed2, i, seed3);
        
        /* Branching pattern */
        int branch = branching_pattern(arith, i, 3 + (i & 3));
        
        /* Assembly barrier operations */
        int barrier = asm_barrier_ops(arith, branch, i);
        
        /* Combine results */
        sum += barrier ^ (i * 1103515245);
        
        /* Occasionally reset pattern */
        if ((i & 15) == 0) {
            asm volatile ("# LOOP RESET BARRIER" ::: "memory");
        }
    }
    
    return sum;
}

/* Main driver that creates multiple scheduling contexts */
int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant propagation */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Execute multiple iterations to increase chance of context creation */
    for (int outer = 0; outer < 100; ++outer) {
        volatile int iter = 5 + (outer & 7); /* Vary iteration count */
        
        /* Call different patterns to engage different scheduler behaviors */
        
        /* 1. Vector operations for target-specific scheduling */
        if (outer & 1) {
            __m128 vec_a = _mm_set1_ps(seed1 * 0.01f);
            __m128 vec_b = _mm_set1_ps(seed2 * 0.02f);
            __m128 vec_c = _mm_set1_ps(seed3 * 0.03f);
            __m128 vec_res = vector_operations(vec_a, vec_b, vec_c);
            
            /* Extract result to use it */
            float res_arr[4];
            _mm_store_ps(res_arr, vec_res);
            total += (int)res_arr[0] + (int)res_arr[1];
        }
        
        /* 2. Mixed loop operations */
        int loop_res = mixed_loop_operations(iter);
        total += loop_res;
        
        /* 3. Direct dense arithmetic */
        int dense_res = dense_arithmetic(seed1 + outer, seed2 - outer, 
                                        seed3 ^ outer, outer);
        total += dense_res;
        
        /* 4. Branching pattern */
        int branch_res = branching_pattern(total, outer, 4 + (outer & 3));
        total ^= branch_res;
        
        /* 5. Assembly barrier operations */
        int asm_res = asm_barrier_ops(total, seed1, seed2);
        total += asm_res;
        
        /* Modify seeds to change patterns */
        seed1 ^= outer * 1664525;
        seed2 += outer * 1013904223;
        seed3 = seed3 * 1103515245 + 12345;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
