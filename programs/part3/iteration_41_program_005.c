/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 42;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    int local_array[256];
    
    /* Initialize local array with volatile pattern */
    for (int i = 0; i < 256; i++) {
        local_array[i] = global_seed + i;
    }
    
    /* Loop to create multiple uses of candidates */
    volatile int loop_cond = 1;
    for (int iter = 0; iter < 100 && loop_cond; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        int cand1 = arg1 + 10;          /* Simple arithmetic - strong candidate */
        int cand2 = arg2 * 2;           /* Another simple recomputable value */
        /* Address calculation with constant offset - good candidate */
        int *cand3 = &local_array[arg3 + 5];
        int cand4 = arg4 - 7;           /* More simple arithmetic */
        
        /* Immediate use of candidates in block A */
        result += cand1;
        result += *cand3;
        result += cand2 + cand4;
        
        /* Control flow to split live ranges */
        volatile int branch_cond = global_seed > 0;  /* Always true but opaque */
        if (branch_cond) {
            /* BLOCK B: High register pressure region */
            /* Many distinct variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            long t3 = t2 * 3L;
            long t4 = t3 - arg2;
            float t5 = t4 * 0.5f;
            float t6 = t5 + arg3;
            double t7 = t6 * 1.5;
            double t8 = t7 - arg4;
            int t9 = (int)t8;
            int t10 = t9 ^ arg1;
            long t11 = t10 | arg2;
            float t12 = t11 * 0.25f;
            double t13 = t12 + t7;
            int t14 = (int)t13;
            int t15 = t14 & 0xFF;
            int t16 = t15 << 2;
            long t17 = t16 * 5L;
            float t18 = t17 * 0.1f;
            double t19 = t18 + t13;
            int t20 = (int)t19;
            
            /* Vector operations to increase register pressure (if available) */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {t1, t2, t3, t4};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            t20 += vsum;
            #else
            /* Fallback: more scalar operations */
            int f1 = t1 + t2 + t3 + t4;
            int f2 = f1 * t5;
            int f3 = f2 / (t6 + 1);
            t20 += f3;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to keep them live */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
            result += t11 + t12 + t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
            
            /* BLOCK C: Use candidate values again after high pressure region */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand1 * 2;        /* Use cand1 again */
            result += cand2 - 5;        /* Use cand2 again */
            result += *cand3 + 10;      /* Use cand3 again */
            result += cand4 / 2;        /* Use cand4 again */
            
            /* More operations to ensure values are used in multiple contexts */
            int tmp1 = cand1 + cand2;
            int tmp2 = cand4 - (int)cand3;
            result += tmp1 * tmp2;
        } else {
            /* Unreachable but prevents optimization */
            result += arg1 * arg2 * arg3 * arg4;
        }
        
        /* Modify volatile condition to prevent loop unrolling */
        if (iter % 10 == 0) {
            global_seed++;
        }
    }
    
    return result;
}

/* Additional function to create cross-function optimization context */
static volatile int __attribute__((noinline))
helper_func(volatile int x, volatile int y) {
    /* Create more rematerialization opportunities */
    int a = x + y;
    int b = x - y;
    int c = x * y;
    int d = (x << 3) | (y & 0xF);
    
    /* High register pressure in helper */
    int r1 = a + b;
    int r2 = c - d;
    long r3 = r1 * r2;
    float r4 = r3 * 0.5f;
    double r5 = r4 + a;
    int r6 = (int)r5;
    
    asm volatile("" ::: "memory");
    
    return r1 + r2 + r3 + r4 + r5 + r6 + a + b + c + d;
}

int main(int argc, char **argv) {
    volatile int total = 0;
    volatile int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Create varying inputs to prevent constant propagation */
    volatile int base = global_seed;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments each iteration */
        volatile int arg1 = base + i;
        volatile int arg2 = base - i * 2;
        volatile int arg3 = base + i * 3;
        volatile int arg4 = base - i * 4;
        
        /* Call test function */
        int result = test_remat(arg1, arg2, arg3, arg4);
        
        /* Also call helper to create more context */
        int helper_result = helper_func(arg1, arg3);
        
        total += result + helper_result;
        
        /* Prevent optimization */
        if (i % 7 == 0) {
            global_seed ^= result;
        }
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
