/* test_sched_context.c - Comprehensive test for Haifa scheduler context cleanup */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>  /* SSE intrinsics */
#include <emmintrin.h>  /* SSE2 intrinsics */

/* Helper function with pure computation - creates scheduling barriers */
static int pure_helper(int a, int b, int c) {
    return (a * b) + (c << 2) - (a ^ b) + (c & 0xFF);
}

/* Function with dense arithmetic sequence - fills instruction queue */
static int dense_arithmetic(int a, int b, int c, int d) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Create many independent operations to fill scheduler structures */
    t1 = a + b;
    t2 = c * d;
    t3 = t1 ^ t2;
    t4 = t1 | t2;
    t5 = t3 & t4;
    t6 = t5 << 3;
    t7 = t6 >> 1;
    t8 = t7 + a;
    t9 = t8 - b;
    t10 = t9 * c;
    t11 = t10 / (d | 1);
    t12 = t11 ^ t1;
    t13 = t12 | t2;
    t14 = t13 & t3;
    t15 = t14 + t4;
    t16 = t15 - t5;
    t17 = t16 * t6;
    t18 = t17 >> 2;
    t19 = t18 | t7;
    t20 = t19 ^ t8;
    
    /* Create some dependencies to force ordering */
    t1 = t20 + t9;
    t2 = t1 * t10;
    t3 = t2 - t11;
    t4 = t3 ^ t12;
    
    return t4 + t13 + t14 + t15;
}

/* Function with SSE intrinsics - triggers target-specific scheduling */
static float sse_computation(float a, float b, float c, float d) {
    __m128 vec1, vec2, vec3, result;
    
    /* Create vector operations that use SSE units */
    vec1 = _mm_set_ps(a, b, c, d);
    vec2 = _mm_set_ps(d, c, b, a);
    vec3 = _mm_set_ps(1.0f, 2.0f, 3.0f, 4.0f);
    
    /* Mix of SSE operations */
    result = _mm_add_ps(vec1, vec2);
    result = _mm_mul_ps(result, vec3);
    result = _mm_sub_ps(result, vec1);
    
    /* Extract result */
    float res[4];
    _mm_storeu_ps(res, result);
    return res[0] + res[1] + res[2] + res[3];
}

/* Function with inline assembly barriers - forces state restoration */
static int asm_barrier_computation(int a, int b, int c) {
    int res1, res2, res3;
    
    /* First computation block */
    asm volatile ("addl %1, %0" : "=r" (res1) : "r" (a), "0" (b));
    
    /* Memory barrier that scheduler might try to move across */
    asm volatile ("" : : : "memory");
    
    /* Second computation - scheduler might try speculative motion */
    asm volatile ("imull %1, %0" : "=r" (res2) : "r" (c), "0" (res1));
    
    /* Another barrier */
    asm volatile ("" : : : "memory");
    
    /* Final computation */
    asm volatile ("xorl %1, %0" : "=r" (res3) : "r" (res2), "0" (0x55AA));
    
    return res3;
}

/* Function with loop and volatile - creates multiple scheduling contexts */
static int loop_with_contexts(volatile int iter, int a, int b) {
    int sum = 0;
    
    /* Loop with volatile iteration count prevents unrolling */
    for (int i = 0; i < iter; ++i) {
        int temp;
        
        /* Inline assembly with dependencies */
        asm volatile ("movl %1, %0\n\t"
                      "addl %2, %0\n\t"
                      "imull $0x1234, %0" 
                      : "=r" (temp) 
                      : "r" (a), "r" (b + i));
        
        /* Call to pure function creates scheduling barrier */
        sum += pure_helper(temp, b, i);
        
        /* Branch with unpredictable outcome */
        if (temp & 1) {
            /* Different computation path */
            asm volatile ("rorl $8, %0" : "+r" (temp));
            sum -= temp;
        } else {
            asm volatile ("roll $4, %0" : "+r" (temp));
            sum += temp * 2;
        }
    }
    
    return sum;
}

/* Function mixing all patterns */
static int mixed_computation(volatile int flag, int a, int b, int c, int d) {
    int result = 0;
    
    /* Branch that might cause speculative scheduling */
    if (flag & 0x1) {
        result += dense_arithmetic(a, b, c, d);
        
        /* SSE computation in one path */
        float fres = sse_computation(a, b, c, d);
        result += (int)fres;
    } else {
        result += asm_barrier_computation(a, b, c);
    }
    
    /* Always execute this with inline assembly */
    int asm_res;
    asm volatile ("/* Start complex sequence */\n\t"
                  "movl %1, %0\n\t"
                  "addl %2, %0\n\t"
                  "subl %3, %0\n\t"
                  "xorl %4, %0\n\t"
                  "/* End sequence */"
                  : "=r" (asm_res)
                  : "r" (a), "r" (b), "r" (c), "r" (d));
    
    result ^= asm_res;
    
    /* Memory operations to introduce load/store dependencies */
    volatile int mem_var = result;
    int mem_read = mem_var;
    
    /* More arithmetic to keep scheduler busy */
    for (int i = 0; i < (flag & 0x3); ++i) {
        mem_read = (mem_read * 1103515245 + 12345) & 0x7FFFFFFF;
        result += (mem_read % 100);
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    
    int total = 0;
    
    /* Main loop - each iteration may create different scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Vary the computation based on seeds to create different paths */
        volatile int iter_count = (seed1 + outer) % 10 + 5;
        
        /* Call different computation patterns */
        total += loop_with_contexts(iter_count, seed1 + outer, seed2 - outer);
        
        total += mixed_computation(seed3 ^ outer, 
                                  seed1 + total, 
                                  seed2 - total, 
                                  seed3 + outer, 
                                  total & 0xFF);
        
        /* Dense computation every few iterations */
        if (outer % 7 == 0) {
            total += dense_arithmetic(total, seed1, seed2, seed3);
        }
        
        /* SSE computation periodically */
        if (outer % 5 == 0) {
            float fval = sse_computation(total * 0.1f, 
                                        (seed1 + outer) * 0.2f,
                                        (seed2 - outer) * 0.3f,
                                        seed3 * 0.4f);
            total += (int)fval;
        }
        
        /* Barrier computation with inline asm */
        total += asm_barrier_computation(total, seed1 ^ outer, seed2 + outer);
        
        /* Pure function calls create scheduling barriers */
        total = pure_helper(total, seed3, outer);
    }
    
    /* Final mixed computation with all patterns */
    total += mixed_computation(total, seed1, seed2, seed3, total);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total & 0xFF;
}
