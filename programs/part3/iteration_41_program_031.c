/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3;
static volatile int vol_result = 0;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates that become "old remats" */
static volatile int test_remat(volatile int a, volatile int b, volatile int c) {
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    
    /* Local array for address calculation candidate */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* REMATERIALIZATION CANDIDATES - simple recomputable values */
    /* Candidate 1: constant derived from argument */
    int cand1 = a + 5;  /* Simple arithmetic - strong remat candidate */
    
    /* Candidate 2: address calculation with constant offset */
    int *cand2 = &local_array[b + 3];  /* Stack address calculation */
    
    /* Candidate 3: arithmetic on volatile parameter */
    int cand3 = c * 2;  /* Simple multiplication */
    
    /* Candidate 4: another constant expression */
    int cand4 = (a << 2) | 0x7;
    
    /* Initial use of candidates (creates initial remat decisions) */
    int sum1 = cand1 + *cand2 + cand3 + cand4;
    
    /* Control flow to split live ranges */
    if (vol_cond) {  /* Always true but opaque to compiler */
        /* BLOCK B: High register pressure region */
        /* Many independent operations to consume registers */
        temp1 = a * b + c;
        temp2 = b * c + a;
        temp3 = c * a + b;
        temp4 = temp1 * temp2;
        temp5 = temp2 * temp3;
        temp6 = temp3 * temp1;
        temp7 = temp4 + temp5;
        temp8 = temp5 + temp6;
        temp9 = temp6 + temp4;
        temp10 = temp7 * temp8;
        temp11 = temp8 * temp9;
        temp12 = temp9 * temp7;
        temp13 = temp10 + temp11;
        temp14 = temp11 + temp12;
        temp15 = temp12 + temp10;
        
        /* Floating point operations */
        ftemp1 = a * 1.5f;
        ftemp2 = b * 2.5f;
        ftemp3 = c * 3.5f;
        ftemp4 = ftemp1 + ftemp2;
        ftemp5 = ftemp2 + ftemp3;
        
        /* Double precision */
        dtemp1 = a * 1.5;
        dtemp2 = b * 2.5;
        dtemp3 = c * 3.5;
        dtemp4 = dtemp1 + dtemp2 + dtemp3;
        
        /* Long integers */
        ltemp1 = (long)a * 1000L;
        ltemp2 = (long)b * 2000L;
        ltemp3 = (long)c * 3000L;
        ltemp4 = ltemp1 + ltemp2;
        ltemp5 = ltemp3 + ltemp4;
        
#ifdef __SSE2__
        /* Vector operations for additional register pressure */
        v4si vec1 = {a, b, c, a+b};
        v4si vec2 = {b, c, a, b+c};
        v4si vec3 = {c, a, b, c+a};
        v4si vec4 = vec1 + vec2;
        v4si vec5 = vec2 + vec3;
        v4si vec6 = vec3 + vec1;
        v4si vec7 = vec4 * vec5;
        v4si vec8 = vec5 * vec6;
        v4si vec9 = vec6 * vec4;
        
        v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
        v4sf fvec2 = {ftemp2, ftemp3, ftemp4, ftemp5};
        v4sf fvec3 = fvec1 + fvec2;
        v4sf fvec4 = fvec1 * fvec2;
        
        /* Use vectors to prevent elimination */
        temp1 += vec4[0] + vec5[1] + vec6[2];
        ftemp1 += fvec3[0] + fvec4[1];
#endif
        
        /* Memory clobber to force spills */
        asm volatile("" ::: "memory");
        
        /* More operations after memory clobber */
        temp1 = temp1 * 2 + temp2;
        temp2 = temp2 * 3 + temp3;
        temp3 = temp3 * 4 + temp4;
        ftemp1 = ftemp1 * 1.1f + ftemp2;
        ftemp2 = ftemp2 * 1.2f + ftemp3;
        dtemp1 = dtemp1 * 1.01 + dtemp2;
        dtemp2 = dtemp2 * 1.02 + dtemp3;
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This should trigger filter_old_remats as original remat decisions
           may no longer be profitable due to register pressure */
        int sum2 = cand1 + *cand2 + cand3 + cand4;
        int sum3 = cand1 * 2 + *cand2 - cand3 + cand4;
        
        /* Use all temporaries to prevent elimination */
        vol_result = sum1 + sum2 + sum3 + temp1 + temp2 + temp3 + temp15 +
                    (int)ftemp1 + (int)ftemp5 + (int)dtemp1 + (int)dtemp4 +
                    (int)ltemp1 + (int)ltemp5;
    }
    
    return vol_result;
}

/* Main function with loop to increase analysis opportunities */
int main(int argc, char **argv) {
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    /* Initialize volatile arguments */
    vol_arg1 = 10;
    vol_arg2 = 20;
    vol_arg3 = 30;
    
    int total = 0;
    
    /* Loop to create more optimization context */
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        vol_arg1 = (vol_arg1 * 13 + 7) & 0xFF;
        vol_arg2 = (vol_arg2 * 17 + 11) & 0xFF;
        vol_arg3 = (vol_arg3 * 19 + 13) & 0xFF;
        
        /* Call test function */
        total += test_remat(vol_arg1, vol_arg2, vol_arg3);
        
        /* Modify condition occasionally */
        if (i % 100 == 0) {
            vol_cond = !vol_cond;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
