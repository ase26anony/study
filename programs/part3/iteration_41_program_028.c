/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 42;

/* Vector extensions for register pressure */
#ifdef __SSE2__
#include <emmintrin.h>
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local variables for register pressure */
    int temp1, temp2, temp3, temp4, temp5, temp6, temp7, temp8;
    int temp9, temp10, temp11, temp12, temp13, temp14, temp15;
    float ftemp1, ftemp2, ftemp3, ftemp4, ftemp5;
    double dtemp1, dtemp2, dtemp3, dtemp4;
    long ltemp1, ltemp2, ltemp3, ltemp4, ltemp5;
    
    /* Stack array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i + global_seed;
    }
    
    volatile int loop_counter = arg1;
    volatile int result = 0;
    
    /* Main loop to create multiple uses of candidates */
    while (loop_counter-- > 0) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + arg2 + 5;  /* arg1 + arg2 + 5 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg3 % 50];  /* &local_array[arg3 % 50] */
        
        /* Candidate 3: More complex but still recomputable */
        int cand3 = (arg4 * 2) - (arg1 / 2);
        
        /* Candidate 4: Pointer arithmetic */
        int *cand4 = local_array + (arg2 % 30);
        
        /* Immediate use in BLOCK A */
        result += *cand2;
        result += cand1;
        result += cand3;
        result += *cand4;
        
        /* Control flow to split live ranges */
        volatile int always_true = global_seed > 0;
        
        if (always_true) {  /* BLOCK B: High register pressure region */
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* High register pressure sequence - many independent operations */
            temp1 = arg1 * arg2;
            temp2 = arg2 * arg3;
            temp3 = arg3 * arg4;
            temp4 = arg4 * arg1;
            temp5 = temp1 + temp2;
            temp6 = temp3 + temp4;
            temp7 = temp5 - temp6;
            temp8 = temp7 * 2;
            temp9 = temp8 / 3;
            temp10 = temp9 << 2;
            temp11 = temp10 >> 1;
            temp12 = temp11 | 0xFF;
            temp13 = temp12 & 0x0F;
            temp14 = temp13 ^ 0x55;
            temp15 = ~temp14;
            
            /* Floating point operations */
            ftemp1 = arg1 * 1.5f;
            ftemp2 = arg2 * 2.5f;
            ftemp3 = arg3 * 3.5f;
            ftemp4 = arg4 * 4.5f;
            ftemp5 = ftemp1 + ftemp2 + ftemp3 + ftemp4;
            
            /* Double precision */
            dtemp1 = arg1 * 1.234567;
            dtemp2 = arg2 * 2.345678;
            dtemp3 = arg3 * 3.456789;
            dtemp4 = arg4 * 4.567890;
            
            /* Long integers */
            ltemp1 = (long)arg1 * 1000000L;
            ltemp2 = (long)arg2 * 2000000L;
            ltemp3 = (long)arg3 * 3000000L;
            ltemp4 = (long)arg4 * 4000000L;
            ltemp5 = ltemp1 + ltemp2 + ltemp3 + ltemp4;
            
#ifdef __SSE2__
            /* Vector operations for additional register pressure */
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {arg2, arg3, arg4, arg1};
            v4si v3 = {arg3, arg4, arg1, arg2};
            v4si v4 = {arg4, arg1, arg2, arg3};
            v4si v5 = v1 + v2;
            v4si v6 = v3 - v4;
            v4si v7 = v5 * v6;
            v4si v8 = v7 + v1;
            v4si v9 = v8 - v2;
            
            /* Use vector results */
            int vsum = v9[0] + v9[1] + v9[2] + v9[3];
            result += vsum;
#else
            /* Fallback scalar operations if no vector support */
            int vsum = arg1 + arg2 + arg3 + arg4;
            vsum = vsum * 2 - vsum / 2;
            vsum = vsum ^ (vsum << 3);
            result += vsum;
#endif
            
            /* More arithmetic to consume results */
            temp1 = temp15 + (int)ftemp5;
            temp2 = (int)dtemp1 + (int)dtemp2;
            temp3 = (int)(ltemp5 >> 32);
            temp4 = temp1 + temp2 + temp3;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += temp4 + temp5 + temp6 + temp7 + temp8 + temp9 + temp10;
            result += temp11 + temp12 + temp13 + temp14 + temp15;
            result += (int)ftemp1 + (int)ftemp2 + (int)ftemp3 + (int)ftemp4;
            result += (int)dtemp1 + (int)dtemp2 + (int)dtemp3 + (int)dtemp4;
            result += (int)(ltemp1 >> 16) + (int)(ltemp2 >> 16) + 
                     (int)(ltemp3 >> 16) + (int)(ltemp4 >> 16);
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;
        result += *cand2 + 1;
        result += cand3 / 2;
        result += *cand4 * 3;
        
        /* Additional use with different addressing modes */
        result += cand1 + cand3;
        result += *cand2 - *cand4;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r" (result) : : "memory");
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
    
    /* Multiple calls with different volatile arguments */
    for (int i = 0; i < iterations; i++) {
        volatile int arg1 = global_seed + i;
        volatile int arg2 = global_seed * 2 + i;
        volatile int arg3 = global_seed / 2 + i;
        volatile int arg4 = global_seed * 3 + i;
        
        total += test_remat(arg1, arg2, arg3, arg4);
        
        /* Change global seed to affect recomputable expressions */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Also use in a volatile store */
    volatile int final_result = total;
    asm volatile("" : : "r" (final_result) : "memory");
    
    return 0;
}
