/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3) {
    volatile int result = 0;
    int local_array[100];
    
    /* Initialize local array with values */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to encourage rematerialization analysis */
    for (int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile arguments */
        int cand1 = arg1 + 5;  /* arg1 + 5 is cheap to recompute */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;  /* arg2 * 2 is cheap to recompute */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 3];  /* &local_array[arg3+3] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * 3) + (arg2 / 2);
        
        /* Immediate use of candidates in Block A */
        result += cand1;
        result += *cand3;
        result += cand2;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            int t3 = t2 - arg2;
            int t4 = t3 * 3;
            int t5 = t4 / 2;
            long t6 = t5 + 1000L;
            long t7 = t6 * 2L;
            long t8 = t7 - 500L;
            float t9 = (float)t8 * 1.5f;
            float t10 = t9 + 3.14f;
            double t11 = (double)t10 * 2.0;
            double t12 = t11 / 1.618;
            int t13 = (int)t12;
            int t14 = t13 ^ 0xFF;
            int t15 = t14 << 2;
            int t16 = t15 >> 1;
            long t17 = (long)t16 * 7L;
            float t18 = (float)t17 / 3.0f;
            double t19 = (double)t18 * 2.71828;
            
            /* More variables to increase pressure */
            int u1 = arg1 + iter;
            int u2 = arg2 - iter;
            int u3 = u1 * u2;
            int u4 = u3 % 17;
            long u5 = (long)u4 * 11L;
            float u6 = (float)u5 * 0.25f;
            double u7 = (double)u6 + 1.234;
            int u8 = (int)u7;
            int u9 = u8 & 0x7F;
            int u10 = u9 | 0x80;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {1, 2, 3, 4};
            v4si v2 = {5, 6, 7, 8};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback scalar operations */
            int vsum = 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8;
            result += vsum;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent optimization */
            result += t1 + t2 + t3 + t4 + t5 + (int)t6 + (int)t7 + (int)t8;
            result += (int)t9 + (int)t10 + (int)t11 + (int)t12;
            result += t13 + t14 + t15 + t16 + (int)t17 + (int)t18 + (int)t19;
            result += u1 + u2 + u3 + u4 + (int)u5 + (int)u6 + (int)u7 + u8 + u9 + u10;
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This should trigger filter_old_remats */
            result += cand1 * 2;
            result += cand2 / 2;
            result += *cand3;
            result += cand4 - 1;
            
            /* Additional use to ensure validate_change has valid insn */
            int final_use = cand1 + cand2 + *cand3 + cand4;
            result += final_use;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(result) : : "memory");
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int total = 0;
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize volatile arguments */
    vol_arg1 = 42;
    vol_arg2 = 17;
    vol_arg3 = 8;
    
    /* Call test function multiple times */
    for (int i = 0; i < iterations; i++) {
        vol_arg1 = (vol_arg1 * 13 + 7) & 0xFF;
        vol_arg2 = (vol_arg2 * 17 + 11) & 0xFF;
        vol_arg3 = (vol_arg3 * 19 + 13) % 50;
        
        total += test_remat(vol_arg1, vol_arg2, vol_arg3);
        
        /* Prevent optimization across iterations */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    printf("Result: %d\n", total);
    return 0;
}
