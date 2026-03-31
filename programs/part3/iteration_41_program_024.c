/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_seed = 42;

/* Function with complex control flow to split live ranges */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    /* Many local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* These are cheap recomputable expressions */
        int cand1 = arg1 + 10;              /* Simple constant addition */
        int cand2 = arg2 * 2;               /* Simple multiplication */
        int cand3 = arg3 & 0xFF;            /* Mask operation */
        int *cand4 = &local_array[arg1];    /* Address calculation with constant offset */
        int cand5 = (arg1 << 3) | (arg2 >> 1); /* Bit operations */
        
        /* Immediate use of candidates in BLOCK A */
        temp1 = cand1 * 3;
        temp2 = cand2 + cand3;
        temp3 = *cand4 + cand5;
        result += temp1 + temp2 + temp3;
        
        /* Conditional jump based on volatile to split control flow */
        /* This creates the "old remats" that need filtering */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            /* Dense sequence of independent operations */
            
            /* Integer operations */
            temp1 = arg1 * iter;
            temp2 = arg2 + iter;
            temp3 = arg3 - iter;
            temp4 = arg1 ^ arg2;
            temp5 = arg2 | arg3;
            temp6 = arg3 & arg1;
            temp7 = temp1 * temp2;
            temp8 = temp3 + temp4;
            temp9 = temp5 - temp6;
            temp10 = temp7 ^ temp8;
            temp11 = temp9 | temp10;
            temp12 = temp11 & iter;
            temp13 = temp12 << 2;
            temp14 = temp13 >> 1;
            temp15 = temp14 + global_seed;
            
            /* Long operations */
            ltemp1 = (long)arg1 * iter;
            ltemp2 = (long)arg2 + iter;
            ltemp3 = (long)arg3 - iter;
            ltemp4 = ltemp1 ^ ltemp2;
            ltemp5 = ltemp3 | ltemp4;
            
            /* Float operations */
            ftemp1 = (float)arg1 * 1.5f;
            ftemp2 = (float)arg2 / 2.0f;
            ftemp3 = ftemp1 + ftemp2;
            ftemp4 = ftemp3 * (float)iter;
            ftemp5 = ftemp4 - 3.14159f;
            
            /* Double operations */
            dtemp1 = (double)arg3 * 2.71828;
            dtemp2 = (double)iter / 3.14159;
            dtemp3 = dtemp1 + dtemp2;
            dtemp4 = dtemp3 * (double)arg1;
            dtemp5 = dtemp4 - dtemp2;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, iter};
            v4si v2 = {iter, arg1, arg2, arg3};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 & v4;
            v4si v6 = v5 | v1;
            
            /* Use vector results */
            int vtemp[4];
            memcpy(vtemp, &v6, sizeof(vtemp));
            temp15 += vtemp[0] + vtemp[1] + vtemp[2] + vtemp[3];
            #else
            /* Fallback scalar operations */
            temp15 += arg1 * 7 + arg2 * 11 + arg3 * 13 + iter * 17;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after clobber */
            temp1 = temp15 * 2;
            temp2 = temp1 + global_seed;
            temp3 = temp2 ^ 0xABCD;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This requires the compiler to either rematerialize or replace */
            int use1 = cand1 + temp3;      /* cand1 needs to be available here */
            int use2 = cand2 * temp2;      /* cand2 needs to be available here */
            int use3 = cand3 | temp1;      /* cand3 needs to be available here */
            int use4 = *cand4 + use1;      /* cand4 needs to be available here */
            int use5 = cand5 ^ use2;       /* cand5 needs to be available here */
            
            result += use1 + use2 + use3 + use4 + use5;
            
            /* More register pressure */
            ftemp1 = (float)use1 * 0.5f;
            dtemp1 = (double)use2 * 1.5;
            result += (int)(ftemp1 + dtemp1);
        }
        
        /* Additional control flow variation */
        if (iter % 2) {
            /* Another use of candidates in different block */
            result += cand1 - cand2 + cand3;
        }
    }
    
    return result;
}

/* Main function with loop to ensure optimization context */
int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile long total = 0;
    
    /* Multiple calls with different arguments */
    for (volatile int i = 0; i < iterations; i++) {
        total += test_remat(
            global_seed + i,
            global_seed - i,
            global_seed * (i + 1),
            (i % 10) + 5
        );
        
        /* Modify global seed to change recomputable expressions */
        global_seed += 1;
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %ld\n", total);
    
    /* Also use the result in a way that can't be optimized away */
    if (total > 1000000) {
        return 1;
    }
    return 0;
}
