/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_base = 100;

/* Function to test early rematerialization */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = arg2 + 5;  /* arg2 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg3 * 2;  /* arg3 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4 % 50];  /* &local_array[arg4 % 50] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 + arg3) * 3 - arg4;
        
        /* Immediate use of candidates in BLOCK A */
        temp1 = cand1 * 2;
        temp2 = cand2 + cand1;
        temp3 = *cand3 + cand4;
        result += temp1 + temp2 + temp3;
        
        /* Conditional jump based on volatile to split control flow */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense independent arithmetic operations */
            temp1 = arg1 * 3;      temp2 = arg2 * 4;
            temp3 = arg3 * 5;      temp4 = arg4 * 6;
            temp5 = temp1 + 7;     temp6 = temp2 + 8;
            temp7 = temp3 + 9;     temp8 = temp4 + 10;
            temp9 = temp5 * 11;    temp10 = temp6 * 12;
            temp11 = temp7 * 13;   temp12 = temp8 * 14;
            temp13 = temp9 + 15;   temp14 = temp10 + 16;
            temp15 = temp11 + 17;
            
            ltemp1 = (long)temp1 * 1000L;   ltemp2 = (long)temp2 * 2000L;
            ltemp3 = (long)temp3 * 3000L;   ltemp4 = (long)temp4 * 4000L;
            ltemp5 = ltemp1 + ltemp2 + ltemp3 + ltemp4;
            
            ftemp1 = (float)temp5 * 1.1f;   ftemp2 = (float)temp6 * 1.2f;
            ftemp3 = (float)temp7 * 1.3f;   ftemp4 = (float)temp8 * 1.4f;
            ftemp5 = ftemp1 + ftemp2 + ftemp3 + ftemp4;
            
            dtemp1 = (double)temp9 * 1.01;   dtemp2 = (double)temp10 * 1.02;
            dtemp3 = (double)temp11 * 1.03;  dtemp4 = (double)temp12 * 1.04;
            dtemp3 = dtemp1 + dtemp2 + dtemp3 + dtemp4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {temp1, temp2, temp3, temp4};
            v4si v2 = {temp5, temp6, temp7, temp8};
            v4si v3 = {temp9, temp10, temp11, temp12};
            v4si v4 = v1 + v2;
            v4si v5 = v3 * v4;
            v4si v6 = v5 - v1;
            
            /* Use vector results */
            int vtemp[4];
            memcpy(vtemp, &v6, sizeof(v6));
            result += vtemp[0] + vtemp[1] + vtemp[2] + vtemp[3];
            #else
            /* Fallback: more scalar operations */
            temp1 = temp1 * temp2 + temp3 * temp4;
            temp2 = temp5 * temp6 + temp7 * temp8;
            temp3 = temp9 * temp10 + temp11 * temp12;
            temp4 = temp13 * temp14 + temp15;
            result += temp1 + temp2 + temp3 + temp4;
            #endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            dtemp1 = dtemp1 * 2.0 + dtemp2;
            dtemp2 = dtemp3 * 3.0 + dtemp4;
            ftemp1 = ftemp1 * 4.0f + ftemp2;
            ftemp2 = ftemp3 * 5.0f + ftemp4;
            ltemp1 = ltemp1 * 6L + ltemp2;
            ltemp2 = ltemp3 * 7L + ltemp4;
            
            result += (int)(dtemp1 + dtemp2 + ftemp1 + ftemp2 + ltemp1 + ltemp2);
        }
        
        /* BLOCK C: Use candidates again after high-pressure region */
        /* This forces compiler to either rematerialize or replace old candidates */
        temp1 = cand1 + iter;          /* Use cand1 */
        temp2 = cand2 * iter;          /* Use cand2 */
        temp3 = *cand3 + iter;         /* Use cand3 (dereference) */
        temp4 = cand4 - iter;          /* Use cand4 */
        
        /* Additional computations mixing candidates */
        temp5 = (cand1 + cand2) * (cand4 % 10);
        temp6 = (*cand3) * cand1 + cand2;
        
        result += temp1 + temp2 + temp3 + temp4 + temp5 + temp6;
        
        /* Another conditional to create more control flow complexity */
        if (always_true) {
            /* Use candidates in different context */
            temp7 = cand1 * cand4;
            temp8 = cand2 + *cand3;
            result += temp7 - temp8;
        }
    }
    
    return result;
}

/* Wrapper function to create cross-function optimization opportunities */
static volatile int remat_wrapper(volatile int x1, volatile int x2,
                                  volatile int x3, volatile int x4) {
    volatile int r1 = test_remat(x1, x2, x3, x4);
    volatile int r2 = test_remat(x4, x3, x2, x1);
    return r1 + r2 * 3;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Multiple calls with different arguments */
    for (volatile int i = 0; i < iterations; i++) {
        total += remat_wrapper(
            vol_arg_base + i,
            vol_arg_base + i * 2,
            vol_arg_base + i * 3,
            vol_arg_base + i * 4
        );
        
        /* Alternate argument patterns */
        total += test_remat(
            i % 10 + 5,
            i % 20 + 10,
            i % 30 + 15,
            i % 40 + 20
        );
    }
    
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    volatile int final_check = total;
    if (final_check == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
