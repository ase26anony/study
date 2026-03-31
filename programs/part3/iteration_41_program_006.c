/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c -o test */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects test.c -o test */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force values to be recomputable but not constant-folded */
static volatile int always_true = 1;
static volatile int global_seed = 42;

/* Vector extensions for register pressure */
#ifdef __SSE2__
#include <emmintrin.h>
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3) {
    volatile int result = 0;
    int local_array[100];
    
    /* Initialize local array */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 3;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < 5; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Simple recomputable expressions - strong remat candidates */
        int cand1 = arg1 + 10;           /* arg + constant */
        int cand2 = arg2 * 2;            /* arg * constant */
        int cand3 = arg3 + arg1;         /* arg + arg */
        
        /* Address calculation with constant offset */
        int *cand4 = &local_array[arg2 + 5];
        
        /* Immediate use in block A */
        result += cand1;
        result += *cand4;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many distinct variables to consume registers */
            int t1 = arg1 * 3;
            int t2 = arg2 + t1;
            int t3 = arg3 - t2;
            long t4 = t1 * 5L;
            long t5 = t2 + t4;
            long t6 = t3 * t5;
            float t7 = t1 * 1.5f;
            float t8 = t2 + t7;
            float t9 = t3 - t8;
            double t10 = t4 * 0.25;
            double t11 = t5 + t10;
            double t12 = t6 * t11;
            int t13 = t1 ^ t2;
            int t14 = t3 | t13;
            int t15 = t14 & t1;
            long t16 = t4 << 2;
            long t17 = t5 >> 1;
            long t18 = t6 ^ t16;
            
            /* More variables to increase pressure */
            int t19 = t13 + t14;
            int t20 = t15 * t19;
            float t21 = t7 + t8;
            float t22 = t9 * t21;
            double t23 = t10 - t11;
            double t24 = t12 + t23;
            
            /* Vector operations for SSE targets */
            #ifdef __SSE2__
            v4si v1 = {t1, t2, t3, t19};
            v4si v2 = {t13, t14, t15, t20};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v3;
            v4si v5 = v2 - v4;
            /* Use vector results */
            t19 += v3[0] + v4[1] + v5[2];
            #else
            /* Fallback: more scalar operations */
            int t25 = t19 * 7;
            int t26 = t20 + t25;
            float t27 = t21 / 2.0f;
            float t28 = t22 - t27;
            double t29 = t23 * 1.1;
            double t30 = t24 + t29;
            t19 += t25 + t26;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to keep them live */
            result += t1 + t2 + t3 + t15 + t19;
            result += (int)(t7 + t8 + t9 + t21 + t22);
            result += (int)(t10 + t11 + t12 + t23 + t24);
            
            /* BLOCK C: Use candidate values again after high pressure */
            /* This should trigger filter_old_remats */
            result += cand1 * 2;      /* Use cand1 - may need rematerialization */
            result += cand2 + arg3;   /* Use cand2 */
            result += cand3 - arg1;   /* Use cand3 */
            result += *cand4 / 2;     /* Use cand4 */
        }
        
        /* Additional uses in loop to extend live ranges */
        result += cand2;
        result += cand3;
    }
    
    return result;
}

/* Second function with different patterns */
static volatile int __attribute__((noinline))
test_remat2(volatile int a, volatile int b, volatile int c) {
    volatile int res = 0;
    int arr[50];
    
    for (int i = 0; i < 50; i++) {
        arr[i] = i + a;
    }
    
    for (volatile int j = 0; j < 3; j++) {
        /* More remat candidates */
        int cand5 = a + b + 7;
        int cand6 = b * 3 - c;
        int *cand7 = &arr[a + 2];
        
        res += cand5;
        res += *cand7;
        
        if (always_true) {
            /* Another high pressure block */
            int x1 = a * b;
            int x2 = c + x1;
            int x3 = x1 - x2;
            long x4 = x2 * 11L;
            float x5 = x3 * 1.7f;
            double x6 = x4 * 0.33;
            
            /* Chain of operations */
            for (int k = 0; k < 4; k++) {
                x1 = x1 + x2 + x3;
                x2 = x2 * 2 - x1;
                x3 = x3 ^ x2;
            }
            
            asm volatile("" ::: "memory");
            
            res += x1 + x2 + x3 + (int)x5 + (int)x6;
            
            /* Reuse candidates */
            res += cand5 / 2;
            res += cand6 * 3;
            res += *cand7 + 1;
        }
    }
    
    return res;
}

int main(int argc, char **argv) {
    volatile int total = 0;
    volatile int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Call test functions multiple times */
    for (volatile int i = 0; i < iterations; i++) {
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        
        /* Vary arguments to prevent constant propagation */
        volatile int arg1 = global_seed % 100;
        volatile int arg2 = (global_seed >> 8) % 100;
        volatile int arg3 = (global_seed >> 16) % 100;
        
        total += test_remat(arg1, arg2, arg3);
        total += test_remat2(arg2, arg3, arg1);
        
        /* Mix in some direct computation */
        total += arg1 * arg2 + arg3;
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Also use dump to verify early-remat ran */
    #ifdef __GNUC__
    asm volatile("" : "=r"(total) : "0"(total));
    #endif
    
    return total != 0;
}
