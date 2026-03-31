/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_counter = 0;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local_array[64];
    volatile int result = 0;
    
    /* Initialize local array */
    for (int i = 0; i < 64; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Simple recomputable expressions - strong rematerialization candidates */
        int cand1 = arg1 + 10;          /* arg + constant */
        int cand2 = arg2 * 2;           /* arg * constant */
        int cand3 = arg3 & 0xFF;        /* arg & constant mask */
        
        /* Address calculation with constant offset - another good candidate */
        int *cand4 = &local_array[arg1 % 64];
        int *cand5 = &local_array[arg2 % 64 + 5];
        
        /* Immediate use of candidates in block A */
        result += cand1;
        result += cand2;
        result += cand3;
        result += *cand4;
        result += *cand5;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many distinct variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            int t3 = t2 - arg2;
            int t4 = t3 * 3;
            int t5 = t4 / 2;
            long t6 = t5 * 5L;
            long t7 = t6 + arg3;
            long t8 = t7 - 1000L;
            long t9 = t8 * 2L;
            float t10 = (float)t9 * 1.5f;
            float t11 = t10 + (float)arg1;
            float t12 = t11 - (float)arg2;
            double t13 = (double)t12 * 2.5;
            double t14 = t13 + (double)arg3;
            double t15 = t14 - 1000.0;
            
            /* More variables to increase pressure */
            int t16 = t1 ^ t2;
            int t17 = t16 | t3;
            int t18 = t17 & t4;
            long t19 = (long)t18 * t6;
            float t20 = (float)t19 / 3.0f;
            double t21 = (double)t20 * t13;
            
            /* Memory clobber to force spills */
            asm volatile("" : : : "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t16, t17, t18};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            /* Use vector results */
            int *vp = (int*)&v5;
            for (int i = 0; i < 4; i++) {
                result += vp[i];
            }
            #else
            /* Fallback scalar operations */
            result += t1 + t2 + t3 + t4 + t5 + t16 + t17 + t18;
            #endif
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This should trigger filter_old_remats as the original
               rematerialization decisions may no longer be valid */
            result += cand1 * 2;
            result += cand2 / 2;
            result += cand3 | 0xAA;
            result += *cand4 * 3;
            result += *cand5 - 10;
            
            /* More computations to ensure values are live */
            int final1 = cand1 + cand2;
            int final2 = cand3 + *cand4;
            result += final1 + final2;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(result));
    }
    
    return result;
}

/* Main function with loop to ensure coverage */
int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test function multiple times with different arguments */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use volatile arguments to create recomputable expressions */
        total += test_remat(i, i * 2, i * 3, 5);
        total += test_remat(i + 1, i * 3, i * 4, 3);
        
        /* Modify global to prevent optimization */
        global_counter++;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return total != 0 ? 0 : 1;
}
