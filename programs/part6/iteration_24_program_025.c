/* test_sched_context.c - Test program to trigger free_sched_context coverage */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Volatile variables to prevent constant propagation */
volatile int g_seed1, g_seed2, g_seed3;
volatile float g_fseed1, g_fseed2;

/* Helper with arithmetic sequence to create dense instruction block */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1 = a + b;
    int t2 = c * d;
    int t3 = t1 ^ t2;
    int t4 = t1 & t2;
    int t5 = t3 | t4;
    int t6 = t2 - t1;
    int t7 = t5 * t6;
    int t8 = t7 / (t1 + 1);
    int t9 = t8 << 2;
    int t10 = t9 >> 1;
    int t11 = t10 + t3;
    int t12 = t11 - t4;
    int t13 = t12 * t5;
    int t14 = t13 & 0xFF;
    int t15 = t14 | 0x55;
    int t16 = t15 ^ t6;
    int t17 = t16 + t7;
    int t18 = t17 - t8;
    int t19 = t18 * t9;
    int t20 = t19 / (t10 + 1);
    
    /* Memory operations to introduce load/store dependencies */
    volatile int mem1 = t20;
    int t21 = mem1 + t11;
    volatile int mem2 = t21;
    int t22 = mem2 * t12;
    
    return t22;
}

/* Function with inline assembly barriers to force state restoration */
static int asm_barrier_test(int a, int b, int c) {
    int result = 0;
    
    /* First computation block */
    int t1 = a * b + c;
    
    /* Assembly barrier - scheduler may try to move across this */
    asm volatile ("" : : : "memory");
    
    /* Dependent computation */
    int t2 = t1 * 2 - c;
    
    /* Another barrier with register clobber */
    asm volatile ("# barrier" : : : "eax", "ebx", "ecx", "edx", "memory");
    
    /* More computations */
    int t3 = t2 + a - b;
    
    /* Volatile read to create scheduling boundary */
    volatile int vread = g_seed1;
    int t4 = t3 * vread;
    
    /* Final barrier */
    asm volatile ("" : : : "memory");
    
    result = t4 + t2 + t1;
    return result;
}

/* Vector operations to engage target-specific scheduling */
static __m128 vector_ops(__m128 a, __m128 b, __m128 c) {
    __m128 r1, r2, r3, r4;
    
    /* Mix of vector operations */
    r1 = _mm_add_ps(a, b);
    r2 = _mm_mul_ps(r1, c);
    r3 = _mm_sub_ps(r2, a);
    r4 = _mm_add_ps(r3, b);
    
    /* Shuffle to create dependencies */
    r1 = _mm_shuffle_ps(r4, r3, _MM_SHUFFLE(0, 1, 2, 3));
    r2 = _mm_add_ps(r1, r4);
    
    /* Use inline assembly with vector registers */
    asm volatile (
        "addps %1, %0\n\t"
        "mulps %2, %0"
        : "+x"(r2)
        : "x"(r3), "x"(a)
        : "cc"
    );
    
    return r2;
}

/* Loop with unpredictable trip count to create multiple scheduling contexts */
static int variable_loop_test(int base, int max_iter) {
    int total = 0;
    volatile int iter_limit = max_iter;
    
    for (int i = 0; i < iter_limit; ++i) {
        /* Branch with unpredictable outcome */
        if (g_seed2 & (1 << (i & 7))) {
            /* Inline assembly with dependencies */
            int a = base + i;
            int b = i * 2;
            int c;
            asm volatile (
                "imull %1, %2\n\t"
                "addl %3, %2\n\t"
                "movl %2, %0"
                : "=r"(c)
                : "r"(a), "r"(b), "r"(g_seed3)
                : "cc"
            );
            total += c;
        } else {
            /* Different computation path */
            int d = base - i;
            int e;
            asm volatile (
                "xorl %1, %2\n\t"
                "roll $3, %2\n\t"
                "movl %2, %0"
                : "=r"(e)
                : "r"(d), "r"(i)
                : "cc"
            );
            total ^= e;
        }
        
        /* Small pure function call - acts as scheduling barrier */
        total = dense_arithmetic(total, i, base, g_seed1);
    }
    
    return total;
}

/* Function with mixed operations to stress the scheduler */
static int mixed_ops_test(int a, int b, float c, float d) {
    /* Integer operations */
    int i1 = a * b + (a >> 3);
    int i2 = (b << 2) | (a & 0xFF);
    int i3 = i1 ^ i2;
    
    /* Floating point operations */
    float f1 = c * d + (float)a;
    float f2 = d - c * (float)b;
    volatile float fstore = f1 + f2;
    float f3 = fstore * 2.0f;
    
    /* Convert and mix */
    int i4 = (int)f3 + i3;
    
    /* More arithmetic density */
    for (int j = 0; j < 8; ++j) {
        i4 += (a * j) - (b / (j + 1));
        i4 ^= (i3 << j);
    }
    
    /* Final assembly block with multiple constraints */
    int result;
    asm volatile (
        "movl %1, %%eax\n\t"
        "imull %2, %%eax\n\t"
        "addl %3, %%eax\n\t"
        "movl %%eax, %0"
        : "=r"(result)
        : "r"(i4), "r"(a), "r"(b)
        : "%eax", "cc"
    );
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize volatile seeds from argv to prevent constant folding */
    g_seed1 = (argc > 1) ? atoi(argv[1]) : 12345;
    g_seed2 = (argc > 2) ? atoi(argv[2]) : 67890;
    g_seed3 = (argc > 3) ? atoi(argv[3]) : 54321;
    g_fseed1 = (argc > 4) ? (float)atof(argv[4]) : 1.234f;
    g_fseed2 = (argc > 5) ? (float)atof(argv[5]) : 5.678f;
    
    int total = 0;
    
    /* Main loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the iteration patterns */
        volatile int loop_var = (outer % 10) + 5;
        
        /* Call different test functions to exercise various scheduling scenarios */
        
        /* 1. Dense arithmetic block - fills instruction queue */
        total += dense_arithmetic(total, g_seed1, outer, g_seed2);
        
        /* 2. Assembly barrier test - may cause state save/restore */
        total += asm_barrier_test(total, g_seed2, outer);
        
        /* 3. Variable loop with branches - creates multiple contexts */
        total += variable_loop_test(total, loop_var);
        
        /* 4. Mixed operations - stresses scheduler with different op types */
        total += mixed_ops_test(total, g_seed3, g_fseed1, g_fseed2);
        
        /* 5. Vector operations - engages target-specific scheduling hooks */
        if (outer % 3 == 0) {
            __m128 vec_a = _mm_set_ps(g_fseed1, g_fseed2, (float)total, (float)outer);
            __m128 vec_b = _mm_set_ps((float)g_seed1, (float)g_seed2, g_fseed1, g_fseed2);
            __m128 vec_c = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
            
            __m128 vec_result = vector_ops(vec_a, vec_b, vec_c);
            
            /* Extract result to affect total */
            float fresult[4];
            _mm_store_ps(fresult, vec_result);
            total += (int)fresult[0] + (int)fresult[1];
        }
        
        /* Occasionally introduce a function call barrier */
        if (outer % 7 == 0) {
            /* Use rand() to create unpredictable control flow */
            int r = rand() % 100;
            if (r > 50) {
                /* More dense computations */
                for (int k = 0; k < 20; ++k) {
                    total = (total * 1103515245 + 12345) & 0x7fffffff;
                    total ^= (g_seed1 << (k & 15));
                }
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
