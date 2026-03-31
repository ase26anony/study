/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_store;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    int local_array[256] = {0};
    
    /* Create rematerialization candidates with simple recomputable values */
    int cand1 = arg1 + 10;          /* Simple arithmetic */
    int cand2 = arg2 * 2;           /* Another simple recomputation */
    int cand3 = arg3 + arg4;        /* Expression with multiple args */
    
    /* Use address calculation of stack variable as candidate */
    int *cand4 = &local_array[arg1 % 256];
    
    /* Use candidates immediately in first basic block */
    result += cand1;
    result += cand2;
    result += cand3;
    result += *cand4;
    
    /* Control flow to split live ranges */
    if (vol_cond) {  /* Always true but opaque to compiler */
        /* High register pressure block - many independent operations */
        int t1 = arg1 * 3;
        int t2 = arg2 * 5;
        int t3 = arg3 * 7;
        int t4 = arg4 * 11;
        int t5 = t1 + t2;
        int t6 = t3 + t4;
        int t7 = t5 * t6;
        int t8 = arg1 ^ arg2;
        int t9 = arg3 | arg4;
        int t10 = t8 & t9;
        int t11 = t7 - t10;
        int t12 = arg1 << 2;
        int t13 = arg2 >> 1;
        int t14 = t12 | t13;
        int t15 = t11 ^ t14;
        
        /* Floating point operations to consume more registers */
        double f1 = arg1 * 1.5;
        double f2 = arg2 * 2.5;
        double f3 = arg3 * 3.5;
        double f4 = arg4 * 4.5;
        double f5 = f1 + f2;
        double f6 = f3 + f4;
        double f7 = f5 * f6;
        float f8 = arg1 * 0.5f;
        float f9 = arg2 * 1.5f;
        float f10 = f8 + f9;
        
        /* Long operations for different register classes */
        long l1 = arg1 * 100L;
        long l2 = arg2 * 200L;
        long l3 = l1 + l2;
        long l4 = arg3 * 300L;
        long l5 = arg4 * 400L;
        long l6 = l4 + l5;
        long l7 = l3 * l6;
        
        /* Vector operations if available */
        #ifdef __SSE2__
        typedef int v4si __attribute__((vector_size(16)));
        v4si v1 = {arg1, arg2, arg3, arg4};
        v4si v2 = {arg2, arg3, arg4, arg1};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        v4si v5 = v3 & v4;
        /* Use vector results */
        int vsum = v5[0] + v5[1] + v5[2] + v5[3];
        t15 += vsum;
        #endif
        
        /* Memory clobber to force spills */
        asm volatile("" ::: "memory");
        
        /* Use all temporaries to ensure they're live */
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
        result += t11 + t12 + t13 + t14 + t15;
        result += (int)f1 + (int)f2 + (int)f3 + (int)f4;
        result += (int)f5 + (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
        result += (int)(l1 % 1000) + (int)(l2 % 1000) + (int)(l3 % 1000);
        result += (int)(l4 % 1000) + (int)(l5 % 1000) + (int)(l6 % 1000) + (int)(l7 % 1000);
        
        /* Reuse the original candidates after high pressure region */
        result += cand1 * 2;
        result += cand2 * 3;
        result += cand3 * 4;
        result += (int)(cand4 - local_array);
    }
    
    /* Another use of candidates in different context */
    if (always_true) {
        result += cand1 - cand2 + cand3;
    }
    
    return result;
}

/* Main function with loop to increase analysis opportunities */
int main(int argc, char **argv) {
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    volatile int total = 0;
    
    /* Loop to create more optimization context */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        vol_arg_store = i;
        int arg1 = (i * 3) % 100;
        int arg2 = (i * 5) % 100;
        int arg3 = (i * 7) % 100;
        int arg4 = (i * 11) % 100;
        
        /* Make arguments volatile to prevent constant propagation */
        volatile int varg1 = arg1;
        volatile int varg2 = arg2;
        volatile int varg3 = arg3;
        volatile int varg4 = arg4;
        
        total += test_remat(varg1, varg2, varg3, varg4);
    }
    
    printf("Result: %d\n", total);
    return 0;
}
