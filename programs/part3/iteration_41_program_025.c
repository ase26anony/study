/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 12345;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local_array[128];
    volatile int result = 0;
    
    /* Initialize array with volatile pattern */
    for (int i = 0; i < 128; i++) {
        local_array[i] = global_seed + i;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 10;  /* arg1 + 10 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg2 + 5];  /* &local_array[arg2 + 5] */
        
        /* Candidate 3: More complex but still recomputable */
        int cand3 = (arg3 * 2) + (arg1 >> 1);
        
        /* Immediate use of candidates in block A */
        result += *cand2;
        result += cand1;
        result += cand3;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (global_seed > 0) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = result + 1;
            int t2 = t1 * 2;
            int t3 = t2 - arg1;
            int t4 = t3 + arg2;
            int t5 = t4 * 3;
            long t6 = t5 + 1000L;
            long t7 = t6 * 2L;
            long t8 = t7 - 500L;
            float f1 = (float)t8 * 0.5f;
            float f2 = f1 + 1.0f;
            float f3 = f2 * 2.0f;
            double d1 = (double)f3 * 1.5;
            double d2 = d1 + 3.14159;
            double d3 = d2 * 2.0;
            
            /* More variables to increase pressure */
            int t9 = t5 + t3;
            int t10 = t9 * 7;
            int t11 = t10 - arg3;
            int t12 = t11 + 100;
            long t13 = t12 * 3L;
            long t14 = t13 + t8;
            float f4 = (float)t14 * 0.25f;
            float f5 = f4 + f2;
            double d4 = (double)f5 + d3;
            
            /* Even more pressure */
            int t15 = arg1 + arg2 + arg3;
            int t16 = t15 * 11;
            int t17 = t16 - t12;
            int t18 = t17 + t9;
            long t19 = (long)t18 * 5L;
            long t20 = t19 + t14;
            float f6 = (float)t20 * 0.1f;
            float f7 = f6 + f5;
            double d5 = d4 + (double)f7;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6 & 0xFFFF, t7 & 0xFFFF, t8 & 0xFFFF};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * (v4si){2, 3, 4, 5};
            /* Use vector results */
            int vsum = v4[0] + v4[1] + v4[2] + v4[3];
            result += vsum;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent optimization */
            result += t1 + t2 + t3 + t4 + t5 + (t6 & 0xFFFF) + (t7 & 0xFFFF) + (t8 & 0xFFFF);
            result += (int)f1 + (int)f2 + (int)f3;
            result += (int)d1 + (int)d2 + (int)d3;
            result += t9 + t10 + t11 + t12 + (t13 & 0xFFFF) + (t14 & 0xFFFF);
            result += (int)f4 + (int)f5 + (int)d4;
            result += t15 + t16 + t17 + t18 + (t19 & 0xFFFF) + (t20 & 0xFFFF);
            result += (int)f6 + (int)f7 + (int)d5;
            
            /* BLOCK C: Use candidates again after high pressure */
            /* This forces compiler to reconsider rematerialization */
            result += cand1 * 2;
            result += *cand2 * 3;
            result += cand3 * 4;
            
            /* More uses in different contexts */
            if (cand1 > 0) {
                result += cand1 + arg1;
            }
            if (*cand2 != 0) {
                result += *cand2 - arg2;
            }
            result += (cand3 & 0xFF) + (cand1 & 0xFF) + (*cand2 & 0xFF);
        }
        
        /* Additional control flow variation */
        if (iter % 2 == 0) {
            /* Another use of candidates in different block */
            result -= cand1;
            result += *cand2;
        } else {
            result += cand3;
        }
    }
    
    return result;
}

int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Multiple calls with different volatile arguments */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        volatile int arg1 = global_seed + i;
        volatile int arg2 = global_seed * 2 + i;
        volatile int arg3 = global_seed / 2 + i;
        volatile int arg4 = 5 + (i % 3);  /* Small loop count */
        
        total += test_remat(arg1, arg2, arg3, arg4);
        
        /* Modify global seed to change recomputable expressions */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %d\n", total);
    
    /* Dump RTL for analysis */
    #ifdef DUMP_RTL
    extern void dump_rtl(void);
    dump_rtl();
    #endif
    
    return 0;
}
