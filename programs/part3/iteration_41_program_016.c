/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_counter = 0;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    /* Result accumulator */
    volatile int result = 0;
    
    /* Many local variables to increase register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile argument (cheap to recompute) */
        int cand1 = arg2 + 10;  /* arg2 + 10 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg3 + 5];  /* &local_array[arg3 + 5] */
        
        /* Candidate 3: Another arithmetic expression */
        int cand3 = arg4 * 2 + 7;
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 * 3) + (arg3 << 2);
        
        /* Immediate use of candidates in BLOCK A */
        temp1 = cand1 * 2;
        temp2 = *cand2 + cand3;
        temp3 = cand4 - cand1;
        
        result += temp1 + temp2 + temp3;
        
        /* Conditional jump based on volatile to split control flow */
        /* This creates separate basic blocks for the live ranges */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            /* This should cause the compiler to reconsider rematerialization */
            
            /* Many independent arithmetic operations consuming registers */
            temp1 = arg1 * 3;      temp2 = arg2 * 5;
            temp3 = arg3 * 7;      temp4 = arg4 * 11;
            temp5 = temp1 + temp2; temp6 = temp3 - temp4;
            temp7 = temp5 * temp6; temp8 = temp7 >> 2;
            temp9 = temp8 + arg1;  temp10 = temp9 - arg2;
            temp11 = arg3 * temp10; temp12 = arg4 + temp11;
            temp13 = temp12 * 3;   temp14 = temp13 / 2;
            temp15 = temp14 + 1;
            
            /* Long operations */
            ltemp1 = (long)arg1 * 1000L;
            ltemp2 = (long)arg2 * 2000L;
            ltemp3 = ltemp1 + ltemp2;
            ltemp4 = (long)arg3 * 3000L;
            ltemp5 = ltemp3 - ltemp4;
            
            /* Float operations */
            ftemp1 = (float)arg1 * 1.5f;
            ftemp2 = (float)arg2 * 2.5f;
            ftemp3 = ftemp1 + ftemp2;
            ftemp4 = (float)arg3 * 3.5f;
            ftemp5 = ftemp3 - ftemp4;
            
            /* Double operations */
            dtemp1 = (double)arg1 * 1.23456;
            dtemp2 = (double)arg2 * 2.34567;
            dtemp3 = dtemp1 + dtemp2;
            dtemp4 = (double)arg3 * 3.45678;
            dtemp5 = dtemp3 - dtemp4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1, v2, v3, v4, v5;
            v1 = (v4si){arg1, arg2, arg3, arg4};
            v2 = (v4si){5, 6, 7, 8};
            v3 = v1 + v2;
            v4 = v1 * v2;
            v5 = v3 - v4;
            
            /* Use vector results */
            int vtemp[4];
            memcpy(vtemp, &v5, sizeof(v5));
            temp15 += vtemp[0] + vtemp[1] + vtemp[2] + vtemp[3];
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after memory clobber */
            temp1 = temp15 * 2;
            temp2 = temp1 + ltemp5;
            temp3 = temp2 + (int)ftemp5;
            temp4 = temp3 + (int)dtemp5;
            
            result += temp4;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This requires the compiler to either rematerialize or replace */
            int use1 = cand1 + temp4;      /* cand1 used again */
            int use2 = *cand2 * cand3;     /* cand2 and cand3 used again */
            int use3 = cand4 / (use1 + 1); /* cand4 used again */
            
            result += use1 + use2 + use3;
            
            /* More operations to prevent optimization */
            global_counter += iter + use1 + use2 + use3;
        }
        
        /* Alternate path to ensure both branches exist */
        else {
            /* This path should never be taken but must exist */
            result += cand1 + *cand2 + cand3 + cand4;
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
    
    /* Call test_remat multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        /* Use volatile arguments to prevent constant propagation */
        volatile int arg1 = (i % 50) + 10;
        volatile int arg2 = (i % 30) + 5;
        volatile int arg3 = (i % 20) + 3;
        volatile int arg4 = (i % 40) + 7;
        
        total += test_remat(arg1, arg2, arg3, arg4);
    }
    
    printf("Result: %d\n", total);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
