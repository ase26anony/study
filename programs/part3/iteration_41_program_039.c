/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat_lto test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3;

/* Function to create rematerialization candidates and trigger filter_old_remats */
static volatile int __attribute__((noinline))
test_remat(volatile int a, volatile int b, volatile int c, volatile int d) {
    /* Local variables for register pressure */
    int local_array[100];
    volatile int result = 0;
    
    /* Initialize local array */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (int iter = 0; iter < 100; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        int cand1 = a + 5;           /* Constant derived from argument */
        int cand2 = b * 2;           /* Simple arithmetic */
        int cand3 = c + 10;
        
        /* Address calculations - strong rematerialization candidates */
        int *addr1 = &local_array[a % 50];
        int *addr2 = &local_array[b % 50 + 5];
        int *addr3 = &local_array[c % 50 + 10];
        
        /* Immediate use of candidates in BLOCK A */
        result += *addr1 + cand1;
        result += *addr2 + cand2;
        result += *addr3 + cand3;
        
        /* Conditional jump based on volatile to split control flow */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            
            /* Many distinct local variables to increase register pressure */
            int t1 = result * 2;
            int t2 = t1 + a;
            int t3 = t2 * b;
            int t4 = t3 - c;
            int t5 = t4 / (d + 1);
            long t6 = t5 * 100L;
            long t7 = t6 + 500L;
            long t8 = t7 - t6;
            float f1 = t8 * 0.5f;
            float f2 = f1 + 1.5f;
            float f3 = f2 * 2.0f;
            double d1 = f3 * 3.14159;
            double d2 = d1 / 2.71828;
            double d3 = d2 + d1;
            int t9 = (int)d3;
            int t10 = t9 ^ t1;
            int t11 = t10 | t2;
            int t12 = t11 & t3;
            int t13 = t12 << 2;
            int t14 = t13 >> 1;
            int t15 = t14 + t4;
            int t16 = t15 - t5;
            
            /* Additional variables for even more pressure */
            long t17 = t16 * 7L;
            long t18 = t17 + 11L;
            float f4 = t18 * 0.25f;
            float f5 = f4 - 0.75f;
            double d4 = f5 * 1.414;
            double d5 = d4 + 2.718;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t7, t8};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback scalar operations */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
            #endif
            
            /* Use all temporary variables to prevent optimization */
            result += t9 + t10 + t11 + t12 + t13 + t14 + t15 + t16;
            result += (int)t17 + (int)t18 + (int)f4 + (int)f5 + (int)d4 + (int)d5;
            
            /* BLOCK C: Use rematerialization candidates again after high pressure */
            result += cand1 + cand2 + cand3;
            result += *addr1 + *addr2 + *addr3;
            
            /* Additional uses with different expressions */
            result += (cand1 * 2) + (cand2 / 2) + (cand3 + 5);
            result += addr1[0] + addr2[1] + addr3[2];
        }
        
        /* Alternate path to ensure control flow complexity */
        if (iter % 10 == 0) {
            /* Different computation using candidates */
            result -= cand1 - cand2 + cand3;
            result -= addr1[0] - addr2[0] + addr3[0];
        }
    }
    
    return result;
}

/* Second function to create cross-function optimization opportunities */
static volatile int __attribute__((noinline))
helper_function(volatile int x, volatile int y) {
    /* Create more rematerialization candidates */
    int cand4 = x * 3 + 7;
    int cand5 = y / 2 - 5;
    
    /* Local array for address calculations */
    int local[50];
    for (int i = 0; i < 50; i++) {
        local[i] = i + x + y;
    }
    
    int *addr4 = &local[x % 25];
    int *addr5 = &local[y % 25 + 10];
    
    volatile int res = 0;
    
    /* Loop with control flow splits */
    for (int i = 0; i < 50; i++) {
        res += cand4 + cand5;
        res += *addr4 + *addr5;
        
        if (always_true) {
            /* High pressure block */
            int p1 = res * i;
            int p2 = p1 + x;
            int p3 = p2 * y;
            int p4 = p3 - cand4;
            int p5 = p4 / (cand5 + 1);
            asm volatile("" ::: "memory");
            
            res += p1 + p2 + p3 + p4 + p5;
            res += cand4 * 2 + cand5 / 2;
            res += addr4[0] + addr5[1];
        }
    }
    
    return res;
}

int main(int argc, char **argv) {
    /* Read iteration count from command line or use default */
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize volatile arguments with non-constant values */
    vol_arg1 = argc;
    vol_arg2 = iterations;
    vol_arg3 = argc * 2;
    
    volatile int total_result = 0;
    
    /* Main loop to execute test multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        int arg1 = vol_arg1 + i;
        int arg2 = vol_arg2 - i;
        int arg3 = vol_arg3 * (i % 5 + 1);
        int arg4 = (arg1 + arg2 + arg3) % 100;
        
        /* Call test function */
        total_result += test_remat(arg1, arg2, arg3, arg4);
        
        /* Call helper function for LTO opportunities */
        if (i % 3 == 0) {
            total_result += helper_function(arg1, arg2);
        }
        
        /* Modify volatile condition occasionally */
        if (i % 7 == 0) {
            always_true = !always_true;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", total_result);
    
    return 0;
}
