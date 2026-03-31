/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_seed = 12345;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates that become "old remats" */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4, dtemp5;
    
    /* Local array for address calculation candidate */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 5;  /* arg1 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;  /* arg2 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 3];  /* &local_array[arg3 + 3] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg4) + (arg2 / 2);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand2;
        result += cand4;
        
        /* Conditional jump based on volatile to split control flow */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense independent arithmetic operations */
            temp1 = arg1 + arg2;
            temp2 = arg2 + arg3;
            temp3 = arg3 + arg4;
            temp4 = arg4 + arg1;
            temp5 = temp1 * temp2;
            temp6 = temp2 * temp3;
            temp7 = temp3 * temp4;
            temp8 = temp4 * temp1;
            temp9 = temp5 + temp6;
            temp10 = temp6 + temp7;
            temp11 = temp7 + temp8;
            temp12 = temp8 + temp5;
            temp13 = temp9 ^ temp10;
            temp14 = temp10 ^ temp11;
            temp15 = temp11 ^ temp12;
            
            /* Mixed types for more register pressure */
            ltemp1 = (long)temp1 * temp2;
            ltemp2 = (long)temp3 * temp4;
            ltemp3 = (long)temp5 * temp6;
            ltemp4 = (long)temp7 * temp8;
            ltemp5 = ltemp1 + ltemp2 + ltemp3 + ltemp4;
            
            ftemp1 = (float)arg1 / 3.0f;
            ftemp2 = (float)arg2 / 5.0f;
            ftemp3 = (float)arg3 / 7.0f;
            ftemp4 = (float)arg4 / 11.0f;
            ftemp5 = ftemp1 + ftemp2 + ftemp3 + ftemp4;
            
            dtemp1 = (double)arg1 / 13.0;
            dtemp2 = (double)arg2 / 17.0;
            dtemp3 = (double)arg3 / 19.0;
            dtemp4 = (double)arg4 / 23.0;
            dtemp5 = dtemp1 + dtemp2 + dtemp3 + dtemp4;
            
#ifdef __SSE2__
            /* Vector operations for even more register pressure */
            v4si vec1 = {temp1, temp2, temp3, temp4};
            v4si vec2 = {temp5, temp6, temp7, temp8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 + vec4;
            
            v4sf fvec1 = {ftemp1, ftemp2, ftemp3, ftemp4};
            v4sf fvec2 = {ftemp5, ftemp1, ftemp2, ftemp3};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
#endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* More operations to consume results */
            temp1 = temp13 + temp14;
            temp2 = temp14 + temp15;
            temp3 = (int)ltemp5;
            temp4 = (int)(ftemp5 * 100.0f);
            temp5 = (int)(dtemp5 * 100.0);
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand1 * 2;
            result += cand2 / 2;
            result += *cand3 * 3;
            result += cand4 - 1;
            
            /* Use all temporaries to prevent elimination */
            result += temp1 + temp2 + temp3 + temp4 + temp5;
#ifdef __SSE2__
            result += vec5[0] + vec5[1];
            result += (int)fvec3[0] + (int)fvec3[1];
#endif
        } else {
            /* Unreachable but prevents optimization */
            result += arg1 + arg2 + arg3 + arg4;
        }
        
        /* Modify arguments slightly to prevent loop invariant motion */
        arg1 += 1;
        arg2 += 2;
        arg3 = (arg3 + 1) % 50;
        arg4 += 3;
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
    
    volatile long total = 0;
    
    /* Multiple calls with different arguments to create different patterns */
    for (volatile int i = 0; i < iterations; i++) {
        total += test_remat(
            global_seed + i * 7,
            global_seed + i * 11,
            (global_seed + i * 13) % 50,
            global_seed + i * 17
        );
        
        /* Modify global seed to change patterns */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result: %ld\n", total);
    
    /* Use result in a way that prevents dead code elimination */
    if (total > 1000000) {
        printf("Large result detected\n");
    }
    
    return (int)(total % 1000);
}
