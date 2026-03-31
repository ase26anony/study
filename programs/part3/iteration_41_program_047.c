/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_store;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 2;
    }
    
    /* Many local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
#ifdef __SSE2__
    v4si vec1, vec2, vec3, vec4, vec5;
    v4sf fvec1, fvec2, fvec3;
#endif
    
    /* Loop to create multiple uses of remat candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg2 + 5;  /* arg2 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg3 * 2;  /* arg3 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4 & 0xFF];  /* &local_array[arg4] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 * 3) + (arg3 / 2);
        
        /* Immediate use of candidates in BLOCK A */
        temp1 = cand1 * 2;
        temp2 = cand2 + cand1;
        temp3 = *cand3 + cand4;
        
        result += temp1 + temp2 + temp3;
        
        /* Conditional jump based on volatile to split control flow */
        if (vol_cond) {  /* Always true at runtime */
            /* BLOCK B: High register pressure region */
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense sequence of independent operations */
            temp1 = arg1 * 3;      temp2 = arg2 * 4;
            temp3 = arg3 * 5;      temp4 = arg4 * 6;
            temp5 = temp1 + 7;     temp6 = temp2 + 8;
            temp7 = temp3 + 9;     temp8 = temp4 + 10;
            temp9 = temp5 * 11;    temp10 = temp6 * 12;
            temp11 = temp7 * 13;   temp12 = temp8 * 14;
            temp13 = temp9 + 15;   temp14 = temp10 + 16;
            temp15 = temp11 + 17;
            
            ltemp1 = (long)temp1 * 1000L;
            ltemp2 = (long)temp2 * 2000L;
            ltemp3 = (long)temp3 * 3000L;
            ltemp4 = (long)temp4 * 4000L;
            ltemp5 = ltemp1 + ltemp2 + ltemp3 + ltemp4;
            
            ftemp1 = (float)temp5 * 1.1f;
            ftemp2 = (float)temp6 * 2.2f;
            ftemp3 = (float)temp7 * 3.3f;
            ftemp4 = (float)temp8 * 4.4f;
            ftemp5 = ftemp1 + ftemp2 + ftemp3 + ftemp4;
            
            dtemp1 = (double)temp9 * 1.01;
            dtemp2 = (double)temp10 * 2.02;
            dtemp3 = (double)temp11 * 3.03;
            dtemp4 = (double)temp12 * 4.04;
            dtemp3 = dtemp1 + dtemp2 + dtemp3 + dtemp4;
            
#ifdef __SSE2__
            /* Vector operations for additional register pressure */
            vec1 = (v4si){temp1, temp2, temp3, temp4};
            vec2 = (v4si){temp5, temp6, temp7, temp8};
            vec3 = vec1 + vec2;
            vec4 = vec1 * vec2;
            vec5 = vec3 + vec4;
            
            fvec1 = (v4sf){ftemp1, ftemp2, ftemp3, ftemp4};
            fvec2 = (v4sf){1.5f, 2.5f, 3.5f, 4.5f};
            fvec3 = fvec1 * fvec2;
#endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* More operations to consume results */
            temp1 = (int)ltemp5;
            temp2 = (int)ftemp5;
            temp3 = (int)dtemp3;
            
#ifdef __SSE2__
            /* Use vector results */
            temp4 = vec5[0] + vec5[1] + vec5[2] + vec5[3];
            temp5 = (int)fvec3[0] + (int)fvec3[1];
#endif
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This forces compiler to either rematerialize or replace */
            int use1 = cand1 + temp1;      /* cand1 used again */
            int use2 = cand2 + temp2;      /* cand2 used again */
            int use3 = *cand3 + temp3;     /* cand3 used again */
            int use4 = cand4 + temp4;      /* cand4 used again */
            
            /* Additional computation with candidates */
            temp6 = cand1 * cand2;
            temp7 = cand2 + *cand3;
            temp8 = cand3 - cand1;
            temp9 = cand4 / (cand1 + 1);
            
            /* Accumulate into volatile result */
            result += use1 + use2 + use3 + use4 + temp6 + temp7 + temp8 + temp9;
            
            /* More operations to prevent optimization */
            result += (iter & 1) ? temp1 : temp2;
            result += (iter & 2) ? temp3 : temp4;
        }
        
        /* Additional loop-invariant computation to create more candidates */
        if (always_true) {
            /* More rematerialization candidates */
            int cand5 = arg1 + arg2 + 100;
            int cand6 = arg3 * arg4 - 50;
            int *cand7 = &local_array[(arg1 + arg2) & 0xFF];
            
            /* Use them */
            temp10 = cand5 + cand6 + *cand7;
            result += temp10;
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
    
    volatile int final_result = 0;
    
    /* Call test function multiple times with different volatile args */
    for (int i = 0; i < iterations; i++) {
        vol_arg_store = i;
        final_result += test_remat(
            vol_arg_store + 1,    /* arg1 */
            vol_arg_store + 2,    /* arg2 */
            vol_arg_store + 3,    /* arg3 */
            vol_arg_store + 4     /* arg4 */
        );
        
        /* Modify volatile condition occasionally */
        if (i % 10 == 0) {
            vol_cond = !vol_cond;
        }
    }
    
    printf("Result: %d\n", final_result);
    return final_result != 0 ? 0 : 1;
}
