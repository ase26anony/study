/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat_lto test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force values to be recomputable but not constant-folded */
static volatile int global_volatile = 0;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local_array[64];
    volatile int result = 0;
    
    /* Initialize array with values */
    for (int i = 0; i < 64; i++) {
        local_array[i] = i * 2 + arg1;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    volatile int loop_counter = arg4 ? arg4 : 100;
    for (int iter = 0; iter < loop_counter; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Simple recomputable expressions - strong rematerialization candidates */
        int cand1 = arg1 + 10;              /* Constant offset from argument */
        int cand2 = arg2 * 2;               /* Simple arithmetic */
        int cand3 = arg3 + arg1;            /* Two arguments */
        
        /* Address calculation with constant offset - good candidate */
        int *cand4 = &local_array[arg2 % 32];
        int *cand5 = &local_array[arg3 % 32 + 5];
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += cand3;
        result += *cand4;
        result += *cand5;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (global_volatile >= 0) {  /* Always true at runtime */
            /* BLOCK B: High register pressure region */
            /* Many distinct variables to consume registers */
            int t1 = arg1 * 3;
            int t2 = arg2 * 4;
            int t3 = arg3 * 5;
            int t4 = arg1 + arg2;
            int t5 = arg2 + arg3;
            int t6 = arg3 + arg1;
            long t7 = arg1 * 100L;
            long t8 = arg2 * 200L;
            long t9 = arg3 * 300L;
            float t10 = arg1 * 1.5f;
            float t11 = arg2 * 2.5f;
            float t12 = arg3 * 3.5f;
            double t13 = arg1 * 1.25;
            double t14 = arg2 * 2.25;
            double t15 = arg3 * 3.25;
            
            /* Additional variables for more pressure */
            int t16 = t1 + t2;
            int t17 = t3 + t4;
            int t18 = t5 + t6;
            long t19 = t7 + t8;
            long t20 = t9 + t7;
            float t21 = t10 + t11;
            float t22 = t12 + t10;
            double t23 = t13 + t14;
            double t24 = t15 + t13;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg1};
            v4si v2 = {arg2, arg3, arg1, arg2};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 + v4;
            
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback scalar operations */
            int vsum = (arg1 + arg2) * 3 + (arg2 + arg3) * 2;
            result += vsum;
            #endif
            
            /* Use all temporaries to prevent elimination */
            result += t16 + t17 + t18;
            result += (int)(t19 % 1000) + (int)(t20 % 1000);
            result += (int)t21 + (int)t22;
            result += (int)t23 + (int)t24;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand1 * 2;
            result += cand2 / 2;
            result += cand3 - arg1;
            result += *cand4 * 3;
            result += *cand5 / 3;
        } else {
            /* Unreachable but prevents optimization */
            result += arg1 * arg2 * arg3;
        }
        
        /* Modify arguments slightly to prevent loop unrolling */
        arg1 += (iter % 3);
        arg2 += (iter % 5);
        arg3 += (iter % 7);
    }
    
    return result;
}

/* Main function with loop to ensure sufficient execution */
int main(int argc, char **argv)
{
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    volatile int total_result = 0;
    
    /* Call test function multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        /* Use varying arguments to prevent constant propagation */
        volatile int arg1 = i % 100;
        volatile int arg2 = (i + 1) % 100;
        volatile int arg3 = (i + 2) % 100;
        volatile int arg4 = (i + 3) % 100;
        
        total_result += test_remat(arg1, arg2, arg3, arg4);
        
        /* Modify global volatile to affect control flow */
        global_volatile += (i % 10);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return 0;
}
