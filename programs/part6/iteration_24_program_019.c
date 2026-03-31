/* test_sched_context.c - Trigger free_sched_context coverage in haifa-sched.cc */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper function with pure computation to create scheduling regions */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (b | c);
}

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to fill scheduler structures */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = a - b;
    t5 = c + d;
    t6 = t4 | t5;
    t7 = t3 & t6;
    t8 = a << 2;
    t9 = b >> 1;
    t10 = t8 * t9;
    t11 = c ^ d;
    t12 = t10 + t11;
    t13 = t7 - t12;
    t14 = a * c;
    t15 = b * d;
    t16 = t14 + t15;
    t17 = t13 ^ t16;
    t18 = t1 * t5;
    t19 = t2 + t4;
    t20 = t18 - t19;
    
    /* Create dependencies to force ordering */
    t1 = t20 + t17;
    t2 = t1 * t13;
    t3 = t2 ^ t16;
    t4 = t3 | t12;
    t5 = t4 & t10;
    
    return t5 + t6 + t7 + t8 + t9 + t11 + t14 + t15 + t18 + t19;
}

/* Function with inline assembly barriers to cause state restoration */
static int asm_barrier_sequence(int a, int b, int c) {
    int result = 0;
    volatile int barrier = 0;
    
    /* Initial computation */
    result = a * b + c;
    
    /* Assembly barrier that scheduler might try to move across */
    asm volatile ("" : "+r" (result) : : "memory");
    
    /* More computations after barrier */
    result ^= (b << 3);
    result += (c * 7);
    
    /* Another barrier */
    asm volatile ("# barrier %0" : "+r" (result) : : "memory", "cc");
    
    /* Final computations */
    result = (result & 0xFF) | (a << 8);
    
    /* Complex inline assembly with dependencies */
    asm volatile (
        "addl %1, %0\n\t"
        "xorl %2, %0\n\t"
        "shrl $2, %0"
        : "+r" (result)
        : "r" (b), "r" (c)
        : "cc"
    );
    
    return result + barrier; /* Use volatile to prevent optimization */
}

/* Function with vector operations to trigger target-specific scheduling */
static __m128 vector_operations(__m128 a, __m128 b, __m128 c) {
    __m128 t1, t2, t3, t4, t5, t6;
    
    /* Multiple vector operations to engage vector scheduler */
    t1 = _mm_add_ps(a, b);
    t2 = _mm_mul_ps(b, c);
    t3 = _mm_sub_ps(t1, t2);
    t4 = _mm_add_ps(a, c);
    t5 = _mm_mul_ps(t3, t4);
    t6 = _mm_add_ps(t5, _mm_set1_ps(1.0f));
    
    /* Mix with integer vector operations if available */
    __m128i i1 = _mm_set1_epi32(0x7FFFFFFF);
    __m128i i2 = _mm_set1_epi32(0x3F800000);
    
    /* Use inline assembly for specific vector scheduling */
    asm volatile (
        "movaps %1, %%xmm0\n\t"
        "movaps %2, %%xmm1\n\t"
        "addps %%xmm1, %%xmm0\n\t"
        "movaps %%xmm0, %0"
        : "=x" (t1)
        : "x" (t6), "x" (c)
        : "xmm0", "xmm1"
    );
    
    return _mm_add_ps(t1, t6);
}

/* Function with unpredictable branching for speculative scheduling */
static int branching_sequence(volatile int cond, int a, int b, int c) {
    int result = 0;
    
    /* Unpredictable branch */
    if (cond & 1) {
        result = pure_helper(a, b, c);
        
        /* Inline asm that scheduler might speculate across */
        asm volatile ("# branch_true" : : : "memory");
        
        result += dense_arithmetic(a, b, c, cond);
    } else {
        result = asm_barrier_sequence(b, c, a);
        
        /* Different asm pattern */
        asm volatile (
            "imull %1, %0\n\t"
            "addl $0x1234, %0"
            : "+r" (result)
            : "r" (cond)
            : "cc"
        );
    }
    
    /* Another level of branching */
    for (int i = 0; i < (cond & 3); ++i) {
        result ^= (a << i);
        
        /* Memory clobber to limit scheduling */
        asm volatile ("" : "+r" (result) : : "memory");
        
        result += pure_helper(result, b, c);
    }
    
    return result;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    volatile int seed1, seed2, seed3;
    int total = 0;
    
    /* Initialize from argv to prevent constant propagation */
    seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    /* Create volatile pointers for memory operations */
    volatile int *mem1 = (volatile int *)malloc(sizeof(int) * 16);
    volatile int *mem2 = (volatile int *)malloc(sizeof(int) * 16);
    
    if (!mem1 || !mem2) return -1;
    
    /* Initialize memory */
    for (int i = 0; i < 16; i++) {
        mem1[i] = seed1 + i;
        mem2[i] = seed2 - i;
    }
    
    /* Main loop to create multiple scheduling contexts */
    for (int iter = 0; iter < 100; iter++) {
        int local_seed = seed1 + iter;
        volatile int cond = seed2 ^ iter;
        
        /* Pattern 1: Dense arithmetic to fill instruction queue */
        total += dense_arithmetic(local_seed, seed2, seed3, iter);
        
        /* Pattern 2: Branching with unpredictable control flow */
        total += branching_sequence(cond, local_seed, seed2, seed3);
        
        /* Pattern 3: Memory operations with dependencies */
        int mem_result = 0;
        for (int i = 0; i < 8; i++) {
            mem1[i] = mem2[i] + local_seed;
            mem_result += mem1[i];
            
            /* Create load-store dependencies */
            mem2[i] = mem1[i] * 2 - iter;
            mem_result ^= mem2[i];
            
            /* Memory barrier */
            asm volatile ("" : : : "memory");
        }
        total += mem_result;
        
        /* Pattern 4: Vector operations (every 4th iteration) */
        if ((iter & 3) == 0) {
            __m128 v1 = _mm_set_ps(seed1 * 0.1f, seed2 * 0.2f, 
                                  seed3 * 0.3f, iter * 0.4f);
            __m128 v2 = _mm_set_ps(seed2 * 0.5f, seed3 * 0.6f,
                                  seed1 * 0.7f, iter * 0.8f);
            __m128 v3 = _mm_set_ps(seed3 * 0.9f, seed1 * 1.0f,
                                  seed2 * 1.1f, iter * 1.2f);
            
            __m128 vres = vector_operations(v1, v2, v3);
            
            /* Extract result */
            float fres[4];
            _mm_store_ps(fres, vres);
            total += (int)fres[0] + (int)fres[1] + (int)fres[2] + (int)fres[3];
        }
        
        /* Pattern 5: Mixed inline assembly with complex dependencies */
        int asm_result = local_seed;
        for (int j = 0; j < 4; j++) {
            asm volatile (
                "movl %1, %%eax\n\t"
                "imull %%eax, %0\n\t"
                "addl %2, %0\n\t"
                "xorl %%eax, %0"
                : "+r" (asm_result)
                : "r" (seed2 + j), "r" (seed3 - j)
                : "eax", "cc"
            );
            
            /* Schedule barrier */
            asm volatile ("# loop_barrier %0" : "+r" (asm_result) : : "memory");
        }
        total += asm_result;
        
        /* Pattern 6: Function calls with volatile arguments */
        volatile int varg1 = seed1 ^ iter;
        volatile int varg2 = seed2 + iter;
        volatile int varg3 = seed3 - iter;
        
        total += pure_helper(varg1, varg2, varg3);
        total += asm_barrier_sequence(varg2, varg3, varg1);
        
        /* Occasionally flush memory */
        if ((iter & 7) == 0) {
            for (int i = 0; i < 16; i++) {
                mem1[i] = mem1[i] ^ mem2[i];
                asm volatile ("" : : : "memory");
            }
        }
    }
    
    /* Final computation with all patterns combined */
    int final_result = total;
    
    /* Complex final sequence with many scheduling opportunities */
    for (int i = 0; i < 20; i++) {
        final_result = pure_helper(final_result, seed1 + i, seed2 - i);
        
        if (i & 1) {
            asm volatile (
                "roll $3, %0\n\t"
                "notl %0"
                : "+r" (final_result)
                :
                : "cc"
            );
        } else {
            asm volatile (
                "rorl $2, %0\n\t"
                "andl $0xFFFF, %0"
                : "+r" (final_result)
                :
                : "cc"
            );
        }
        
        /* Memory operation to create dependencies */
        mem1[i & 15] = final_result;
        final_result ^= mem2[i & 15];
    }
    
    free((void *)mem1);
    free((void *)mem2);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    return final_result & 0xFF;
}
