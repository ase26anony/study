/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-schedule-insns -fno-tree-vectorize test.c */
/* Additional flags for LTO: -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_store;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local1, local2, local3, local4, local5, local6, local7, local8;
    int local9, local10, local11, local12, local13, local14, local15;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Local array for address calculations */
    int local_array[100];
    
    /* Result accumulator */
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    for (int i = 0; i < 100; i++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = arg1 + 10;  /* arg1 + 10 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;   /* arg2 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 5];  /* &local_array[arg3 + 5] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg2) + (arg3 - arg4);
        
        /* Immediate use of candidates in BLOCK A */
        local_array[i] = cand1 + cand2;
        result += *cand3;
        result += cand4;
        
        /* Conditional jump based on volatile to split control flow */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            
            /* Independent arithmetic operations with many variables */
            local1 = arg1 + i;
            local2 = arg2 - i;
            local3 = arg3 * i;
            local4 = arg4 / (i + 1);
            local5 = local1 + local2;
            local6 = local3 - local4;
            local7 = local5 * local6;
            local8 = local7 + arg1;
            local9 = local8 - arg2;
            local10 = local9 * arg3;
            local11 = local10 / (arg4 + 1);
            local12 = local11 + i * 2;
            local13 = local12 - i / 2;
            local14 = local13 * 3;
            local15 = local14 + 7;
            
            /* Floating point operations */
            f1 = arg1 * 1.5f;
            f2 = arg2 * 2.5f;
            f3 = arg3 * 3.5f;
            f4 = arg4 * 4.5f;
            f5 = f1 + f2 + f3 + f4;
            
            /* Double operations */
            d1 = (double)arg1 / 3.0;
            d2 = (double)arg2 / 4.0;
            d3 = (double)arg3 / 5.0;
            d4 = (double)arg4 / 6.0;
            d5 = d1 * d2 * d3 * d4;
            
            /* Long operations */
            l1 = (long)arg1 << 2;
            l2 = (long)arg2 << 3;
            l3 = (long)arg3 << 4;
            l4 = (long)arg4 << 5;
            l5 = l1 + l2 + l3 + l4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {i, i+1, i+2, i+3};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback: more scalar operations */
            int extra1 = arg1 * i * 11;
            int extra2 = arg2 * i * 13;
            int extra3 = arg3 * i * 17;
            int extra4 = arg4 * i * 19;
            result += extra1 + extra2 + extra3 + extra4;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after clobber */
            local1 = local1 * 2 + 1;
            local2 = local2 * 3 + 2;
            local3 = local3 * 5 + 3;
            f1 = f1 * 1.1f;
            f2 = f2 * 1.2f;
            d1 = d1 * 1.01;
            d2 = d2 * 1.02;
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This should trigger replacement of old remats */
            result += cand1;      /* Use cand1 again */
            result += cand2;      /* Use cand2 again */
            result += *cand3;     /* Use cand3 again */
            result += cand4;      /* Use cand4 again */
            
            /* More uses with different expressions */
            local_array[i + 1] = cand1 - cand2;
            result += cand3 - (int*)local_array;
        } else {
            /* This branch never taken but prevents optimization */
            result += arg1 * 1000;
        }
        
        /* Additional loop-carried pressure */
        vol_arg_store = arg1 + arg2 + arg3 + arg4;
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
    
    volatile long total_result = 0;
    
    /* Multiple calls with different arguments to create various patterns */
    for (volatile int i = 0; i < iterations; i++) {
        /* Use different volatile arguments each iteration */
        volatile int arg1 = i * 3 + 1;
        volatile int arg2 = i * 5 + 2;
        volatile int arg3 = i * 7 + 3;
        volatile int arg4 = i * 11 + 4;
        
        total_result += test_remat(arg1, arg2, arg3, arg4);
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(i) : : "memory");
    }
    
    printf("Result: %ld\n", (long)total_result);
    
    /* Use result to prevent dead code elimination */
    if (total_result > 1000000) {
        return 1;
    }
    return 0;
}
