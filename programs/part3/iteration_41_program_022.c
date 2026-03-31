/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_seed = 42;

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
    
    /* Create rematerialization candidates - simple recomputable values */
    int cand1 = arg1 + 10;           /* Constant offset from argument */
    int cand2 = arg2 * 2;            /* Simple arithmetic */
    int cand3 = arg3 & 0xFF;         /* Mask operation */
    
    /* Address calculation - strong rematerialization candidate */
    int *cand4 = &local_array[arg4 % 64];
    
    /* First use of candidates (creates initial remat candidates) */
    result += cand1;
    result += cand2;
    result += cand3;
    result += *cand4;
    
    /* Control flow split - force live range splitting */
    if (always_true) {  /* Always taken but opaque to compiler */
        /* BLOCK B: High register pressure region */
        
        /* Many distinct local variables to consume registers */
        int t1 = result * 2;
        int t2 = t1 + arg1;
        int t3 = t2 - arg2;
        int t4 = t3 * 3;
        int t5 = t4 / 2;
        long t6 = t5 + 1000L;
        long t7 = t6 * 2L;
        long t8 = t7 - 500L;
        float t9 = t8 * 0.5f;
        float t10 = t9 + 1.0f;
        double t11 = t10 * 2.0;
        double t12 = t11 - 1.0;
        int t13 = (int)t12;
        int t14 = t13 ^ arg3;
        int t15 = t14 | arg4;
        
        /* More variables for additional pressure */
        int t16 = t15 << 2;
        int t17 = t16 >> 1;
        int t18 = t17 + 255;
        int t19 = t18 & 0xFF;
        int t20 = t19 * 7;
        long t21 = t20 + 9999L;
        float t22 = t21 * 0.25f;
        double t23 = t22 + 3.14159;
        
        /* Memory clobber to force spills */
        asm volatile("" ::: "memory");
        
        /* Vector operations if available */
        #ifdef __SSE2__
        typedef int v4si __attribute__((vector_size(16)));
        v4si v1 = {t1, t2, t3, t4};
        v4si v2 = {t5, t6, t7, t8};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        v4si v5 = v3 - v4;
        
        /* Use vector results */
        int vsum = v5[0] + v5[1] + v5[2] + v5[3];
        t23 += vsum;
        #endif
        
        /* More scalar operations as fallback */
        int t24 = (int)t23;
        int t25 = t24 * 11;
        int t26 = t25 % 97;
        int t27 = t26 + arg1;
        int t28 = t27 - arg2;
        int t29 = t28 * 3;
        int t30 = t29 / 2;
        
        /* Use all temporaries to prevent optimization */
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + 
                 (int)t9 + (int)t10 + (int)t11 + (int)t12 +
                 t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20 +
                 t21 + (int)t22 + (int)t23 + t24 + t25 + t26 + 
                 t27 + t28 + t29 + t30;
                 
        #ifdef __SSE2__
        result += vsum;
        #endif
        
        /* Another memory clobber */
        asm volatile("" ::: "memory");
    }
    
    /* BLOCK C: Second use of original candidates after high pressure region */
    /* This forces compiler to reconsider rematerialization */
    result += cand1 * 2;
    result += cand2 / 2;
    result += cand3 | 0x80;
    result += (int)(cand4 - local_array);
    
    /* Additional use in different context */
    int final1 = cand1 + cand2;
    int final2 = cand3 * (*cand4);
    result += final1 + final2;
    
    return result;
}

/* Main function with loop to increase analysis opportunities */
int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile long total = 0;
    
    /* Loop to create more optimization context */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        volatile int arg1 = global_seed + i;
        volatile int arg2 = global_seed * 2 - i;
        volatile int arg3 = global_seed ^ i;
        volatile int arg4 = (global_seed + i * 3) % 64;
        
        /* Call test function */
        int result = test_remat(arg1, arg2, arg3, arg4);
        total += result;
        
        /* Modify global seed to prevent pattern recognition */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    /* Use result in conditional to prevent optimization */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return (int)(total % 1000);
}
