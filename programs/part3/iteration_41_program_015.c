/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;

/* Function to test early rematerialization */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int local1, local2, local3, local4, local5, local6, local7, local8;
    int local9, local10, local11, local12, local13, local14, local15;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Compute rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg2 + 10;  /* arg2 + 10 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg3 * 2;   /* arg3 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4 + 5];  /* &local_array[arg4 + 5] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 * 3) + (arg3 / 2);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand2;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Force spills with memory clobber */
            asm volatile("" ::: "memory");
            
            /* Many independent arithmetic operations */
            local1 = arg1 + arg2;
            local2 = arg1 * arg3;
            local3 = arg2 - arg4;
            local4 = arg3 + arg4;
            local5 = arg1 * arg4;
            local6 = local1 * local2;
            local7 = local3 + local4;
            local8 = local5 - local6;
            local9 = local7 * local8;
            local10 = local9 / (arg1 + 1);
            local11 = local10 + arg2;
            local12 = local11 * arg3;
            local13 = local12 - arg4;
            local14 = local13 + local1;
            local15 = local14 * 2;
            
            /* Floating point operations */
            f1 = (float)arg1 * 1.1f;
            f2 = (float)arg2 * 2.2f;
            f3 = (float)arg3 * 3.3f;
            f4 = (float)arg4 * 4.4f;
            f5 = f1 + f2 + f3 + f4;
            
            /* Double precision operations */
            d1 = (double)arg1 * 1.111;
            d2 = (double)arg2 * 2.222;
            d3 = (double)arg3 * 3.333;
            d4 = (double)arg4 * 4.444;
            d5 = d1 + d2 + d3 + d4;
            
            /* Long operations */
            l1 = (long)arg1 * 1000L;
            l2 = (long)arg2 * 2000L;
            l3 = (long)arg3 * 3000L;
            l4 = (long)arg4 * 4000L;
            l5 = l1 + l2 + l3 + l4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {10, 20, 30, 40};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #endif
            
            /* More operations to increase pressure */
            local1 = local1 + f1;
            local2 = local2 + f2;
            local3 = local3 + f3;
            local4 = local4 + f4;
            local5 = local5 + f5;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* Use all local variables to prevent elimination */
            result += local1 + local2 + local3 + local4 + local5;
            result += local6 + local7 + local8 + local9 + local10;
            result += local11 + local12 + local13 + local14 + local15;
            result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
            result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces compiler to either rematerialize or replace */
        result += cand1 * 2;
        result += cand2 / 2;
        result += *cand3 * 3;
        result += cand4 - 5;
        
        /* Additional use to ensure values are needed */
        if (always_true) {
            result += cand1 + cand2 + cand4;
            result += *cand3;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < iterations; i++) {
        total += test_remat(
            (i % 10) + 1,      /* arg1 */
            (i % 7) + 2,       /* arg2 */
            (i % 5) + 3,       /* arg3 */
            (i % 3) + 4        /* arg4 */
        );
    }
    
    printf("Result: %d\n", total);
    return 0;
}
