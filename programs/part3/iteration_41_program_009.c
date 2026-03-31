/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force values to be recomputable but not constant-folded */
static volatile int always_true = 1;
static volatile int global_seed = 42;

/* Vector extensions for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Local array for address calculation candidates */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int iter = 0; iter < arg1; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg2 + 10;  /* arg2 + 10 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg3 * 2;   /* arg3 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg4];  /* &local_array[arg4] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg2 << 2) + arg3;
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand2;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (always_true) {  /* Always taken but opaque to compiler */
            /* BLOCK B: High register pressure region */
            
            /* Independent arithmetic operations consuming many registers */
            temp1 = arg1 * 3;
            temp2 = arg2 + temp1;
            temp3 = arg3 - temp2;
            temp4 = arg4 * temp3;
            temp5 = temp1 + temp2;
            temp6 = temp3 * temp4;
            temp7 = temp5 - temp6;
            temp8 = temp7 + arg1;
            temp9 = temp8 * 2;
            temp10 = temp9 / 3;
            temp11 = temp10 + arg2;
            temp12 = temp11 - arg3;
            temp13 = temp12 * 4;
            temp14 = temp13 + arg4;
            temp15 = temp14 - temp1;
            
            /* Long operations */
            ltemp1 = (long)temp1 * temp2;
            ltemp2 = (long)temp3 * temp4;
            ltemp3 = ltemp1 + ltemp2;
            ltemp4 = ltemp3 * 5;
            ltemp5 = ltemp4 - (long)temp5;
            
            /* Float operations */
            ftemp1 = (float)arg1 * 1.5f;
            ftemp2 = (float)arg2 + ftemp1;
            ftemp3 = (float)arg3 - ftemp2;
            ftemp4 = (float)arg4 * ftemp3;
            ftemp5 = ftemp1 + ftemp2 + ftemp3 + ftemp4;
            
            /* Double operations */
            dtemp1 = (double)arg1 * 2.5;
            dtemp2 = (double)arg2 + dtemp1;
            dtemp3 = (double)arg3 - dtemp2;
            dtemp4 = (double)arg4 * dtemp3;
            dtemp5 = dtemp1 + dtemp2 + dtemp3 + dtemp4;
            
#ifdef __SSE2__
            /* Vector operations for additional register pressure */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, temp7, temp8};
            v4si vec3 = {temp9, temp10, temp11, temp12};
            v4si vec4 = vec1 + vec2;
            v4si vec5 = vec3 * vec4;
            v4si vec6 = vec5 - vec1;
            
            v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf fvec2 = {1.0f, 2.0f, 3.0f, 4.0f};
            v4sf fvec3 = fvec1 * fvec2;
            v4sf fvec4 = fvec3 + fvec1;
            
            /* Use vector results */
            temp1 += vec4[0];
            temp2 += vec5[1];
            temp3 += vec6[2];
            ftemp1 += fvec3[0];
            ftemp2 += fvec4[1];
#endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations after clobber */
            temp1 = temp1 * 2 + 1;
            temp2 = temp2 / 2 - 1;
            temp3 = temp3 + temp1 - temp2;
            temp4 = temp4 * temp3;
            
            /* Use all temporaries to prevent elimination */
            result += temp1 + temp2 + temp3 + temp4 + temp5 + temp6 + temp7 + temp8 +
                     temp9 + temp10 + temp11 + temp12 + temp13 + temp14 + temp15;
            result += (int)ltemp1 + (int)ltemp2 + (int)ltemp3 + (int)ltemp4 + (int)ltemp5;
            result += (int)ftemp1 + (int)ftemp2 + (int)ftemp3 + (int)ftemp4 + (int)ftemp5;
            result += (int)dtemp1 + (int)dtemp2 + (int)dtemp3 + (int)dtemp4 + (int)dtemp5;
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand1 * 2;
            result += cand2 + 5;
            result += *cand3;
            result += cand4 - 3;
            
            /* Additional use with different computation to ensure validate_change */
            int use1 = cand1 + cand2;
            int use2 = *cand3 + cand4;
            result += use1 * use2;
        }
        
        /* Alternate path to create more control flow complexity */
        if (global_seed > 0) {
            /* Simple computation to keep block alive */
            result += iter * 7;
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
    
    /* Call test function multiple times with different volatile args */
    for (int i = 0; i < iterations; i++) {
        global_seed = i * 3 + 1;
        total += test_remat(
            global_seed % 10 + 1,
            global_seed % 20 + 2,
            global_seed % 30 + 3,
            global_seed % 40 + 4
        );
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Additional compilation options for LTO testing */
    #ifdef TEST_LTO
    /* This code only compiles with LTO flags */
    volatile int lto_test = test_remat(5, 6, 7, 8);
    printf("LTO test: %d\n", lto_test);
    #endif
    
    return 0;
}
