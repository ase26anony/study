/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects test.c */

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

/* Force values to be recomputable but not constant-folded */
static int volatile global_seed = 12345;

/* Test function with rematerialization candidates */
static int volatile __attribute__((noinline))
test_remat(int volatile arg1, int volatile arg2, int volatile arg3) {
    /* Local variables for register pressure */
    int local_array[100];
    int i;
    
    /* Initialize local array */
    for (i = 0; i < 100; i++) {
        local_array[i] = i + global_seed;
    }
    
    /* REMATERIALIZATION CANDIDATES - simple recomputable values */
    int cand1, cand2, cand3;
    int *cand_ptr1, *cand_ptr2;
    
    /* Control flow to split live ranges */
    int volatile result = 0;
    int volatile cond = 1; /* Always true but opaque to compiler */
    
    /* Loop to encourage rematerialization analysis */
    for (i = 0; i < 10; i++) {
        /* BLOCK A: Compute candidate values */
        cand1 = arg1 + 5;              /* Simple arithmetic */
        cand2 = arg2 * 2;              /* Another simple computation */
        cand3 = arg3 & 0xFF;           /* Mask operation */
        cand_ptr1 = &local_array[arg1 + 10]; /* Address calculation */
        cand_ptr2 = &local_array[arg2 * 3];  /* Another address calc */
        
        /* Use candidates immediately in BLOCK A */
        result += cand1;
        result += *cand_ptr1;
        
        /* Conditional jump to split live range (always taken) */
        if (cond) {
            /* BLOCK B: High register pressure region */
            
            /* Many distinct local variables for register pressure */
            int t1 = result + 1, t2 = result + 2, t3 = result + 3;
            long t4 = result * 2L, t5 = result * 3L, t6 = result * 4L;
            float t7 = result * 1.1f, t8 = result * 1.2f, t9 = result * 1.3f;
            double t10 = result * 1.4, t11 = result * 1.5, t12 = result * 1.6;
            int t13, t14, t15, t16, t17, t18, t19, t20;
            
            /* Independent arithmetic operations */
            t13 = t1 * t2 + t3;
            t14 = t2 * t3 - t1;
            t15 = t3 / (t1 ? t1 : 1) + t2;
            t16 = (t4 >> 2) | (t5 << 3);
            t17 = (t6 % 7) ^ t4;
            t18 = (int)t7 + (int)t8;
            t19 = (int)(t9 * 2.0f) - (int)t10;
            t20 = (int)(t11 / 2.0) * (int)(t12 * 1.1);
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t13};
            v4si v2 = {t4, t5, t6, t14};
            v4si v3 = {t7, t8, t9, t15};
            v4si v4, v5, v6;
            
            v4 = v1 + v2;
            v5 = v2 * v3;
            v6 = v4 - v5;
            
            /* Use vector results */
            t20 += v6[0] + v6[1] + v6[2] + v6[3];
            #else
            /* Fallback: more scalar operations */
            int v1 = t1 + t4 + t7;
            int v2 = t2 + t5 + t8;
            int v3 = t3 + t6 + t9;
            t20 += v1 * v2 - v3;
            #endif
            
            /* More operations to increase pressure */
            double d1 = t10 * 1.1, d2 = t11 * 1.2, d3 = t12 * 1.3;
            float f1 = t7 * 0.9f, f2 = t8 * 0.8f, f3 = t9 * 0.7f;
            long l1 = t4 * 5, l2 = t5 * 6, l3 = t6 * 7;
            
            /* Chain computations */
            for (int j = 0; j < 3; j++) {
                d1 = d1 * 0.95 + d2;
                d2 = d2 * 0.96 + d3;
                d3 = d3 * 0.97 + d1;
                f1 = f1 * 0.85f + f2;
                f2 = f2 * 0.86f + f3;
                f3 = f3 * 0.87f + f1;
            }
            
            /* Use all temporaries to prevent elimination */
            result += t13 + t14 + t15 + t16 + t17 + t18 + t19 + t20;
            result += (int)d1 + (int)d2 + (int)d3;
            result += (int)f1 + (int)f2 + (int)f3;
            result += (int)l1 + (int)l2 + (int)l3;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
        }
        
        /* BLOCK C: Use candidate values again after high pressure region */
        result += cand2;              /* Use second candidate */
        result += cand3;              /* Use third candidate */
        result += *cand_ptr2;         /* Use pointer candidate */
        
        /* Additional use with different addressing mode */
        result += cand1 * 2;
        result += cand_ptr1 - local_array;
    }
    
    return result;
}

int main(int argc, char **argv) {
    int volatile iterations = 1;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1;
    }
    
    int volatile total = 0;
    
    /* Call test function multiple times */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        int volatile arg1 = global_seed + i * 3;
        int volatile arg2 = global_seed + i * 5;
        int volatile arg3 = global_seed + i * 7;
        
        total += test_remat(arg1, arg2, arg3);
        
        /* Modify global seed to prevent complete optimization */
        global_seed += 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
