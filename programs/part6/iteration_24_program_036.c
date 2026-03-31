/* test_sched_context.c
 * Designed to trigger free_sched_context uncovered lines in haifa-sched.cc
 * Compile with: gcc -O3 -funroll-loops -fschedule-insns2 -march=native -c test_sched_context.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile variables to prevent constant propagation */
volatile int g_seed1, g_seed2, g_seed3;
volatile int g_iter_count = 100;

/* Helper with pure function calls to create scheduling barriers */
static int helper_pure(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

static float helper_float(float a, float b) {
    return a * b + a / (b + 1.0f);
}

/* Function 1: Mixed integer operations with inline asm barriers */
int func1_mixed_ops(volatile int seed) {
    int a = seed + 1;
    int b = seed * 2;
    int c = seed ^ 0x1234;
    int result = 0;
    
    /* Create artificial scheduling barrier */
    asm volatile ("" : : : "memory");
    
    /* Sequence of dependent operations */
    int t1 = a + b;
    int t2 = b * c;
    int t3 = t1 ^ t2;
    int t4 = t3 - a;
    int t5 = t4 * b;
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* More operations with branching */
    for (int i = 0; i < (seed & 0x3) + 2; ++i) {
        t1 = helper_pure(t1, t2);
        t2 = helper_pure(t2, t3);
        t3 = helper_pure(t3, t4);
        
        /* Volatile read to prevent optimization */
        volatile int v = g_seed2;
        t4 += v;
    }
    
    result = t1 + t2 + t3 + t4 + t5;
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    return result;
}

/* Function 2: Vector operations using SSE intrinsics */
int func2_vector_ops(volatile int seed) {
    /* Create vector data */
    float fa[4] = {seed * 1.0f, seed * 2.0f, seed * 3.0f, seed * 4.0f};
    float fb[4] = {seed * 0.5f, seed * 1.5f, seed * 2.5f, seed * 3.5f};
    float fc[4] = {0};
    
    __m128 vec_a = _mm_loadu_ps(fa);
    __m128 vec_b = _mm_loadu_ps(fb);
    
    /* Sequence of vector operations */
    __m128 vec_c = _mm_add_ps(vec_a, vec_b);
    __m128 vec_d = _mm_mul_ps(vec_a, vec_b);
    __m128 vec_e = _mm_sub_ps(vec_c, vec_d);
    __m128 vec_f = _mm_add_ps(vec_e, _mm_set1_ps(1.0f));
    
    /* Mix with scalar operations */
    int result = 0;
    for (int i = 0; i < 4; ++i) {
        fc[i] = ((float*)&vec_f)[i];
        result += (int)fc[i];
        
        /* Inline asm to create scheduling complexity */
        asm volatile ("# Vector operation mix %0" : : "r"(result));
    }
    
    /* More vector operations in a loop */
    for (int i = 0; i < (seed & 0x7) + 1; ++i) {
        vec_a = _mm_add_ps(vec_a, _mm_set1_ps(i * 0.1f));
        vec_b = _mm_mul_ps(vec_b, _mm_set1_ps(1.1f));
        vec_c = _mm_add_ps(vec_c, vec_a);
        
        /* Memory clobber to force scheduler to consider dependencies */
        asm volatile ("" : : : "memory");
    }
    
    _mm_storeu_ps(fc, vec_c);
    for (int i = 0; i < 4; ++i) {
        result += (int)fc[i];
    }
    
    return result;
}

/* Function 3: Dense arithmetic sequence to fill instruction queue */
int func3_dense_arithmetic(volatile int seed) {
    int vars[32];
    int result = seed;
    
    /* Initialize variables */
    for (int i = 0; i < 32; ++i) {
        vars[i] = seed + i;
    }
    
    /* Dense sequence of independent operations */
    int t1 = vars[0] + vars[1];
    int t2 = vars[2] * vars[3];
    int t3 = vars[4] ^ vars[5];
    int t4 = vars[6] & vars[7];
    int t5 = vars[8] | vars[9];
    int t6 = vars[10] - vars[11];
    int t7 = vars[12] / (vars[13] + 1);
    int t8 = vars[14] << (vars[15] & 0x3);
    int t9 = vars[16] >> (vars[17] & 0x3);
    int t10 = vars[18] % (vars[19] + 1);
    
    /* More operations with mixing */
    int t11 = t1 * t2 + t3;
    int t12 = t4 ^ t5 | t6;
    int t13 = t7 - t8 * t9;
    int t14 = t10 & t11 ^ t12;
    int t15 = t13 | t14 + t1;
    
    /* Memory operations mixed in */
    volatile int* mem_ptr = &g_seed3;
    int mem_val = *mem_ptr;
    
    t1 += mem_val;
    t3 -= mem_val;
    t5 ^= mem_val;
    
    /* Another sequence */
    int t16 = helper_pure(t1, t2);
    int t17 = helper_pure(t3, t4);
    int t18 = helper_pure(t5, t6);
    int t19 = helper_pure(t7, t8);
    int t20 = helper_pure(t9, t10);
    
    /* Complex expression to create many instructions */
    result = t1 + t2 - t3 * t4 / (t5 + 1) + 
             t6 ^ t7 & t8 | t9 << 2 + 
             t10 >> 1 * t11 - t12 + 
             t13 % (t14 + 1) + t15 * 
             t16 / (t17 + 1) + t18 - 
             t19 ^ t20;
    
    /* Branch with unpredictable outcome */
    if (mem_val & 0x1) {
        result += func1_mixed_ops(result);
    } else {
        result -= func2_vector_ops(result);
    }
    
    return result;
}

/* Function 4: Loop with varying trip count and complex body */
int func4_varying_loop(volatile int seed) {
    int acc = 0;
    
    /* Loop with volatile bound to prevent unrolling */
    for (int i = 0; i < (seed & 0xF) + 5; ++i) {
        /* Mix of operations in loop body */
        int a = i * seed;
        int b = i + seed;
        int c = i ^ seed;
        
        /* Function call creates scheduling context */
        int tmp = helper_pure(a, b);
        
        /* Inline asm with dependencies */
        asm volatile ("# Loop body operation %0, %1, %2" 
                     : "+r"(tmp) : "r"(c), "r"(seed));
        
        /* Memory operation */
        volatile int v = g_seed1;
        tmp += v;
        
        /* Conditional with pure function */
        if (tmp & 0x1) {
            tmp = helper_pure(tmp, c);
        } else {
            tmp = helper_pure(c, tmp);
        }
        
        acc += tmp;
        
        /* Another memory clobber */
        asm volatile ("" : : : "memory");
    }
    
    return acc;
}

/* Function 5: Nested loops with mixed operations */
int func5_nested_loops(volatile int seed) {
    int total = 0;
    
    /* Outer loop */
    for (int i = 0; i < 3; ++i) {
        int inner_sum = 0;
        
        /* Inner loop with volatile bound */
        for (int j = 0; j < (g_seed2 & 0x3) + 2; ++j) {
            /* Vector-like operations using floats */
            float f1 = (float)(i * j + seed);
            float f2 = (float)(j * seed - i);
            
            /* Float operations */
            float f3 = helper_float(f1, f2);
            float f4 = helper_float(f2, f1);
            
            /* Convert back to int with operations */
            int i1 = (int)f3;
            int i2 = (int)f4;
            
            /* Mixed operations */
            i1 = i1 * i2 + j;
            i2 = i2 ^ i1 - i;
            
            /* Use helper */
            inner_sum += helper_pure(i1, i2);
            
            /* Occasional memory barrier */
            if ((j & 0x1) == 0) {
                asm volatile ("" : : : "memory");
            }
        }
        
        total += inner_sum;
        
        /* Call another function to create new scheduling region */
        if (i & 0x1) {
            total += func1_mixed_ops(total);
        }
    }
    
    return total;
}

int main(int argc, char** argv) {
    /* Initialize volatile seeds from argv to prevent constant propagation */
    g_seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    g_seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    g_seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    int total_result = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < g_iter_count; ++iter) {
        /* Update seeds to vary behavior */
        g_seed1 ^= iter;
        g_seed2 += iter;
        g_seed3 = g_seed3 * 1103515245 + 12345;
        
        /* Call all functions to create diverse scheduling scenarios */
        total_result += func1_mixed_ops(g_seed1);
        total_result += func2_vector_ops(g_seed2);
        total_result += func3_dense_arithmetic(g_seed3);
        total_result += func4_varying_loop(total_result);
        total_result += func5_nested_loops(total_result);
        
        /* Occasionally add a complex branching pattern */
        if ((iter & 0x7) == 0) {
            volatile int choice = g_seed1;
            if (choice & 0x1) {
                total_result = func1_mixed_ops(total_result);
            } else {
                total_result = func2_vector_ops(total_result);
            }
        }
        
        /* Memory clobber between iterations */
        asm volatile ("" : : : "memory");
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return total_result & 0xFF;
}
