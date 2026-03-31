/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

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
    
    /* Initialize array with volatile pattern */
    for (int i = 0; i < 64; i++) {
        local_array[i] = i + global_volatile;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < 4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* These are cheap to recompute expressions */
        int cand1 = arg1 + 10;              /* Simple arithmetic */
        int cand2 = arg2 * 2;               /* Another simple computation */
        int cand3 = arg3 & 0xFF;            /* Mask operation */
        int *cand4 = &local_array[arg4];    /* Address calculation */
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += cand3;
        result += *cand4;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (global_volatile >= 0) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = arg1 * 3;
            int t2 = arg2 + t1;
            int t3 = arg3 - t2;
            int t4 = arg4 ^ t3;
            long t5 = (long)arg1 * arg2;
            long t6 = t5 + arg3;
            long t7 = t6 - arg4;
            float t8 = (float)arg1 * 1.5f;
            float t9 = t8 + (float)arg2;
            float t10 = t9 * 2.0f;
            double t11 = (double)arg3 / 3.0;
            double t12 = t11 + (double)arg4;
            double t13 = t12 * 1.7;
            int t14 = t1 + t2;
            int t15 = t3 + t4;
            int t16 = t14 * t15;
            long t17 = t5 + t6;
            float t18 = t8 + t9;
            double t19 = t11 + t12;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {t1, t2, t3, t4};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 + v4;
            /* Use vector results */
            int *vp = (int*)&v5;
            t16 += vp[0] + vp[1] + vp[2] + vp[3];
            #else
            /* Fallback: more scalar operations */
            int t20 = t1 * t2 + t3 * t4;
            int t21 = t14 + t15 + t16;
            long t22 = t5 * t6 / 7;
            float t23 = t8 * t9 - t10;
            double t24 = t11 * t12 + t13;
            t16 = t20 + t21 + (int)t22;
            #endif
            
            /* Use all temporaries to prevent elimination */
            result += t1 + t2 + t3 + t4 + (int)t5 + (int)t6 + (int)t7;
            result += (int)t8 + (int)t9 + (int)t10;
            result += (int)t11 + (int)t12 + (int)t13;
            result += t14 + t15 + t16;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* After high pressure, compiler may need to replace old remats */
            result += cand1 * 2;
            result += cand2 / 2;
            result += cand3 | 0x80;
            result += *(cand4 + 1);  /* Different offset */
            
            /* More operations to ensure values are live across blocks */
            int cand5 = arg1 + arg2 + arg3 + arg4;
            result += cand5;
            
            /* Additional pressure */
            for (int j = 0; j < 8; j++) {
                int temp = arg1 + j;
                result += temp * temp;
            }
        }
        
        /* Another use outside the if block */
        result += cand1 + cand2;
    }
    
    return result;
}

/* Main function with loop to increase analysis opportunities */
int main(int argc, char **argv)
{
    volatile int iterations = 4;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 4;
    }
    
    volatile int total = 0;
    
    /* Create different argument patterns */
    for (int i = 0; i < iterations; i++) {
        global_volatile = i;  /* Change volatile to affect recomputation */
        
        /* Call with volatile arguments to prevent constant propagation */
        volatile int arg1 = i * 7 + 1;
        volatile int arg2 = i * 13 + 2;
        volatile int arg3 = i * 19 + 3;
        volatile int arg4 = (i * 11 + 4) & 63;  /* Ensure valid array index */
        
        total += test_remat(arg1, arg2, arg3, arg4);
        
        /* Mix in some different patterns */
        if (i % 2 == 0) {
            total += test_remat(arg2, arg3, arg4, arg1);
        } else {
            total += test_remat(arg4, arg1, arg2, arg3);
        }
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Additional test with different optimization contexts */
    if (iterations > 1) {
        volatile int quick_test = test_remat(1, 2, 3, 4);
        printf("Quick test: %d\n", quick_test);
    }
    
    return 0;
}
