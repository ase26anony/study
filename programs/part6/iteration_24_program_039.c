/* Test program to trigger free_sched_context coverage in haifa-sched.cc */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <xmmintrin.h>
#include <emmintrin.h>

/* Helper functions to create scheduling contexts */

/* Function with dense arithmetic sequence to fill instruction queue */
static int dense_arithmetic(int a, int b, int c, int d, volatile int iter) {
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Create many independent operations to fill scheduler structures */
        t1 = a + b;
        t2 = c * d;
        t3 = t1 ^ t2;
        t4 = a * c + b;
        t5 = b * d - a;
        t6 = t3 & t4;
        t7 = t5 | t6;
        t8 = t1 * t2 + t3;
        t9 = t4 - t5 * t6;
        t10 = t7 ^ t8 & t9;
        t11 = t10 * 31 + 17;
        t12 = t11 - t1 * t2;
        t13 = t12 | t3 & t4;
        t14 = t5 + t6 * t7;
        t15 = t8 - t9 ^ t10;
        t16 = t11 * t12 + t13;
        t17 = t14 - t15 & t16;
        t18 = t17 | t13 ^ t14;
        t19 = t15 * t16 + t17;
        t20 = t18 - t19 & t20;
        
        /* Mix in memory operations */
        volatile int mem1 = t1;
        volatile int mem2 = t2;
        t1 = mem1 + mem2;
        
        /* Create artificial scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* More operations after barrier */
        t2 = t3 * t4 + t5;
        t3 = t6 - t7 ^ t8;
        t4 = t9 & t10 | t11;
        
        result += t1 + t2 + t3 + t4 + t20;
    }
    return result;
}

/* Function with vector operations to trigger target-specific scheduling */
static int vector_operations(volatile int seed1, volatile int seed2) {
    __m128i vec1, vec2, vec3, vec4;
    __m128 fvec1, fvec2, fvec3;
    int results[4];
    int total = 0;
    
    for (int i = 0; i < (seed1 & 0x3F) + 10; ++i) {
        /* Mix integer and floating-point vector operations */
        vec1 = _mm_set1_epi32(seed1 + i);
        vec2 = _mm_set1_epi32(seed2 - i);
        vec3 = _mm_add_epi32(vec1, vec2);
        vec4 = _mm_mullo_epi32(vec1, vec2);
        
        /* Switch to floating point */
        fvec1 = _mm_set1_ps((float)(seed1 + i));
        fvec2 = _mm_set1_ps((float)(seed2 - i));
        fvec3 = _mm_add_ps(fvec1, fvec2);
        
        /* Back to integer */
        vec1 = _mm_sub_epi32(vec3, vec4);
        
        /* Store results to force scheduling around memory ops */
        _mm_storeu_si128((__m128i*)results, vec1);
        
        /* Inline assembly with specific constraints */
        asm volatile (
            "movdqa %1, %%xmm0\n\t"
            "paddd %2, %%xmm0\n\t"
            "movdqa %%xmm0, %0\n\t"
            : "=m" (results)
            : "m" (vec1), "m" (vec2)
            : "xmm0"
        );
        
        total += results[0] + results[1] + results[2] + results[3];
        
        /* Branch with unpredictable outcome */
        if (seed1 & (1 << (i & 7))) {
            asm volatile ("" : : : "memory");
            total += i * 3;
        }
    }
    return total;
}

/* Function with complex control flow and scheduling barriers */
static int control_flow_scheduling(int a, int b, volatile int limit) {
    int x = a, y = b;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Multiple independent computation paths */
        int path1 = x * y + i;
        int path2 = x - y * i;
        int path3 = (x ^ y) | i;
        int path4 = (x & y) + (i << 2);
        
        /* Artificial scheduling barrier */
        asm volatile ("" : : : "memory");
        
        /* Conditional that creates need for state restoration */
        if (path1 > path2) {
            /* Complex sequence that might be scheduled speculatively */
            int t1 = path1 * 31;
            int t2 = path2 * 17;
            asm volatile (
                "addl %1, %0\n\t"
                "imull $19, %0, %0\n\t"
                : "+r" (t1)
                : "r" (t2)
            );
            sum += t1;
            
            /* Another barrier */
            asm volatile ("" : : : "memory");
            
            x = t1 ^ path3;
        } else {
            /* Alternative path with different operations */
            int t1 = path3 + path4;
            int t2 = path1 - path2;
            asm volatile (
                "subl %1, %0\n\t"
                "xorl %2, %0\n\t"
                : "+r" (t1)
                : "r" (t2), "r" (path4)
            );
            sum += t1 * 2;
            
            y = t1 & path4;
        }
        
        /* More operations to keep scheduler busy */
        x = (x * 1103515245 + 12345) & 0x7fffffff;
        y = (y * 1664525 + 1013904223) & 0x7fffffff;
        
        /* Function call as scheduling barrier */
        sum += (x > y) ? x % 100 : y % 100;
    }
    return sum;
}

/* Pure function that can be speculated around */
static int pure_helper(int a, int b) {
    return a * b + (a ^ b) - (a & b);
}

/* Function mixing pure calls and volatile accesses */
static int mixed_calls(volatile int a, volatile int b, volatile int iter) {
    int result = 0;
    
    for (int i = 0; i < iter; ++i) {
        /* Volatile reads create scheduling constraints */
        int local_a = a;
        int local_b = b;
        
        /* Multiple pure function calls */
        int r1 = pure_helper(local_a, local_b);
        int r2 = pure_helper(local_b, local_a);
        int r3 = pure_helper(r1, r2);
        int r4 = pure_helper(i, local_a + local_b);
        
        /* Scheduling barrier between dependent operations */
        asm volatile ("" : : : "memory");
        
        /* More computations */
        r1 = r1 * 3 + r2;
        r2 = r2 / 2 + r3;
        r3 = r3 ^ r4 + i;
        
        /* Memory operation */
        volatile int mem = r1;
        r4 = mem + r2 * r3;
        
        result += r1 + r2 + r3 + r4;
        
        /* Modify volatiles to prevent optimization */
        a = a + 1;
        b = b - 1;
    }
    return result;
}

int main(int argc, char *argv[]) {
    /* Use argv for volatile initialization to prevent constant propagation */
    volatile int seed1 = argc > 1 ? atoi(argv[1]) : 12345;
    volatile int seed2 = argc > 2 ? atoi(argv[2]) : 67890;
    volatile int seed3 = argc > 3 ? atoi(argv[3]) : 54321;
    volatile int seed4 = argc > 4 ? atoi(argv[4]) : 98765;
    
    int total = 0;
    
    /* Loop to create multiple scheduling contexts */
    for (int outer = 0; outer < 100; ++outer) {
        /* Call different patterns to exercise various scheduler paths */
        total += dense_arithmetic(seed1 + outer, seed2 - outer, 
                                 seed3 + outer * 2, seed4 - outer * 2,
                                 (outer & 0xF) + 5);
        
        total += vector_operations(seed1 + outer * 3, seed2 + outer * 7);
        
        total += control_flow_scheduling(seed3 + outer, seed4 - outer,
                                        (outer & 0x7) + 3);
        
        total += mixed_calls(seed1 + outer * 11, seed2 + outer * 13,
                            (outer & 0x3) + 2);
        
        /* Modify seeds to create varying patterns */
        seed1 = (seed1 * 1103515245 + 12345) & 0x7fffffff;
        seed2 = (seed2 * 1664525 + 1013904223) & 0x7fffffff;
        seed3 = seed3 ^ (seed1 << 16);
        seed4 = seed4 ^ (seed2 >> 16);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    return total & 0xFF;
}
