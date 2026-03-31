/* test_sched_context.c - Trigger Haifa scheduler context allocation/freeing */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile seeds to prevent constant propagation */
static volatile int seed1, seed2, seed3;
static volatile int *volatile ptr_seed;

/* Helper with scheduling barriers */
static int helper_with_barrier(int a, int b) {
    int t1, t2, t3;
    /* Create artificial scheduling barrier */
    asm volatile ("" : : : "memory");
    t1 = a * b + seed1;
    /* Another barrier to force context save/restore */
    asm volatile ("" : : : "memory");
    t2 = t1 ^ (a << 3);
    t3 = t2 | (b >> 2);
    asm volatile ("" : : : "memory");
    return t3;
}

/* Dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a << 2;
    t5 = b >> 1;
    t6 = t3 + t4;
    t7 = t5 * seed2;
    t8 = t6 - t7;
    t9 = t8 | 0x7F;
    t10 = t9 & 0xFF;
    
    t11 = t10 * 3;
    t12 = t11 / 2;
    t13 = t12 + c;
    t14 = t13 - d;
    t15 = t14 ^ a;
    t16 = t15 | b;
    t17 = t16 << 1;
    t18 = t17 >> 2;
    t19 = t18 + seed3;
    t20 = t19 * 2;
    
    /* Mix with memory operations */
    volatile int mem1 = t20;
    volatile int mem2 = mem1 + 1;
    t20 = mem2 * 3;
    
    return t20;
}

/* Vector operations for target-specific scheduling */
static __m128 vector_ops(__m128 a, __m128 b, __m128 c) {
    __m128 t1, t2, t3, t4, t5, t6;
    
    /* Multiple vector operations to engage vector scheduler */
    t1 = _mm_add_ps(a, b);
    t2 = _mm_mul_ps(t1, c);
    t3 = _mm_sub_ps(b, a);
    t4 = _mm_add_ps(t2, t3);
    t5 = _mm_mul_ps(t4, _mm_set1_ps(2.0f));
    t6 = _mm_add_ps(t5, _mm_set1_ps(1.0f));
    
    /* Inline asm with vector registers */
    asm volatile (
        "addps %1, %0\n\t"
        "mulps %2, %0"
        : "+x"(t6)
        : "x"(a), "x"(b)
        : /* No clobbers - let compiler manage */
    );
    
    return t6;
}

/* Complex loop with unpredictable branches */
static int branching_pattern(int limit) {
    int sum = 0;
    volatile int cond = seed1;
    
    for (int i = 0; i < limit; ++i) {
        /* Unpredictable branch */
        if (cond & (1 << (i & 7))) {
            /* Inline asm with dependencies */
            int a = i * 3;
            int b = i + seed2;
            int result;
            asm volatile (
                "addl %1, %0\n\t"
                "imull %2, %0"
                : "=r"(result)
                : "r"(a), "r"(b), "0"(i)
                : "cc"
            );
            sum += result;
        } else {
            /* Different operation mix */
            sum += helper_with_barrier(i, seed3);
        }
        
        /* Modify condition unpredictably */
        cond ^= (i * 0x1234567);
    }
    
    return sum;
}

/* Mixed integer/float operations */
static float mixed_operations(int a, float b) {
    float f1, f2, f3, f4;
    int i1, i2, i3;
    
    /* Interleave integer and float ops */
    i1 = a * 2;
    f1 = b * 3.14f;
    i2 = i1 + (int)f1;
    f2 = (float)i2 * 1.5f;
    
    /* Memory barrier between types */
    asm volatile ("" : : : "memory");
    
    i3 = i2 ^ 0xABCD;
    f3 = f2 + (float)i3;
    f4 = f3 * 0.5f;
    
    return f4;
}

/* Main test driver */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Initialize volatile seeds from argv to prevent optimization */
    seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    
    /* Allocate volatile memory */
    ptr_seed = (volatile int*)malloc(sizeof(int));
    *ptr_seed = seed1 ^ seed2;
    
    /* Create multiple scheduling contexts through varied patterns */
    for (int iter = 0; iter < 100; ++iter) {
        volatile int loop_var = iter;
        
        /* Pattern 1: Dense arithmetic (fills instruction queue) */
        total += dense_arithmetic(seed1 + iter, seed2 - iter, 
                                 seed3, *ptr_seed);
        
        /* Pattern 2: Vector operations (triggers target hooks) */
        __m128 vec_a = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
        __m128 vec_b = _mm_set_ps(5.0f, 6.0f, 7.0f, 8.0f);
        __m128 vec_c = _mm_set_ps(9.0f, 10.0f, 11.0f, 12.0f);
        __m128 vec_result = vector_ops(vec_a, vec_b, vec_c);
        
        /* Extract result from vector */
        float vec_sum = 0;
        float temp[4];
        _mm_store_ps(temp, vec_result);
        for (int i = 0; i < 4; ++i) {
            vec_sum += temp[i];
        }
        total += (int)vec_sum;
        
        /* Pattern 3: Complex branching */
        total += branching_pattern(20 + (iter % 10));
        
        /* Pattern 4: Mixed operations */
        float float_res = mixed_operations(seed1 + loop_var, 
                                         (float)seed2 * 0.1f);
        total += (int)float_res;
        
        /* Pattern 5: Inline asm with memory clobbers */
        int asm_result;
        asm volatile (
            "movl %1, %%eax\n\t"
            "imull %2, %%eax\n\t"
            "addl %3, %%eax\n\t"
            "movl %%eax, %0"
            : "=r"(asm_result)
            : "r"(seed1), "r"(seed3), "r"(iter)
            : "%eax", "cc"
        );
        total += asm_result;
        
        /* Modify seeds to change scheduling patterns */
        seed1 ^= iter * 0x1111;
        seed2 += iter * 0x2222;
        seed3 = (seed3 << 3) | (seed3 >> 29);
    }
    
    free((void*)ptr_seed);
    
    /* Ensure result is used */
    printf("Total: %d\n", total);
    return total & 0xFF;
}
