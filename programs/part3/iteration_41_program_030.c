/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3, vol_arg4;
static volatile int vol_result = 0;

/* Function to create rematerialization candidates and trigger filter_old_remats */
static volatile int __attribute__((noinline))
test_remat(volatile int a, volatile int b, volatile int c, volatile int d)
{
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 2;
    }
    
    /* Many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Initialize some values */
    v1 = a + 1; v2 = b + 2; v3 = c + 3; v4 = d + 4;
    f1 = a * 1.5f; f2 = b * 2.5f;
    d1 = c * 3.5; d2 = d * 4.5;
    l1 = a * 10L; l2 = b * 20L;
    
    /* Result accumulator */
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (int iteration = 0; iteration < 100; iteration++) {
        /* BLOCK A: Create rematerialization candidates */
        /* These are cheap recomputable expressions */
        int cand1 = a + 5;              /* Simple arithmetic on volatile arg */
        int cand2 = b * 2;              /* Another simple expression */
        int *cand3 = &local_array[a];   /* Address calculation */
        int cand4 = (c << 2) + d;       /* Bit operation and addition */
        
        /* Use candidates immediately in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        result += cand4;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* This should cause reconsideration of rematerialization decisions */
            
            /* Many independent arithmetic operations */
            v5 = v1 + v2; v6 = v3 + v4;
            v7 = v5 * v6; v8 = v7 - v1;
            v9 = v8 / (v2 + 1); v10 = v9 ^ v3;
            v11 = v10 << 2; v12 = v11 >> 1;
            v13 = v12 & 0xFF; v14 = v13 | 0x55;
            v15 = v14 + v4; v16 = v15 - v1;
            v17 = v16 * 3; v18 = v17 / 2;
            v19 = v18 % 17; v20 = v19 ^ 0xAA;
            
            /* Floating point operations */
            f3 = f1 + f2; f4 = f3 * 2.0f; f5 = f4 / 1.5f;
            
            /* Double precision operations */
            d3 = d1 + d2; d4 = d3 * 3.14159; d5 = d4 / 2.71828;
            
            /* Long operations */
            l3 = l1 + l2; l4 = l3 * 5L; l5 = l4 / 3L;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available (consumes more registers) */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si vec1 = {v1, v2, v3, v4};
            v4si vec2 = {v5, v6, v7, v8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec3 * vec1;
            /* Use vector results */
            int vec_sum = vec4[0] + vec4[1] + vec4[2] + vec4[3];
            result += vec_sum;
            #else
            /* Fallback scalar operations */
            int scalar_sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
            result += scalar_sum;
            #endif
            
            /* More operations to increase pressure */
            for (int i = 0; i < 8; i++) {
                v1 = v1 * 1103515245 + 12345;
                v2 = v2 * 1103515245 + 12345;
                v3 = v3 * 1103515245 + 12345;
                v4 = v4 * 1103515245 + 12345;
            }
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This requires compiler to either rematerialize or replace */
        result += cand1 * 2;
        result += cand2 / 2;
        result += *cand3 + 1;
        result += cand4 - 1;
        
        /* Additional use to ensure multiple references */
        if (iteration % 2 == 0) {
            result += cand1 + cand2;
        } else {
            result += *cand3 - cand4;
        }
    }
    
    /* Final computation using all variables */
    int final = result + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
                + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return final;
}

int main(int argc, char **argv)
{
    /* Read iteration count from command line or use default */
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize volatile arguments with non-constant values */
    vol_arg1 = argc;
    vol_arg2 = argc * 2;
    vol_arg3 = argc * 3;
    vol_arg4 = argc * 4;
    
    /* Accumulate results to prevent dead code elimination */
    volatile int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Modify arguments slightly each iteration */
        vol_arg1 += 1;
        vol_arg2 += 2;
        vol_arg3 += 3;
        vol_arg4 += 4;
        
        /* Call test function */
        int result = test_remat(vol_arg1, vol_arg2, vol_arg3, vol_arg4);
        total += result;
        
        /* Change volatile condition occasionally */
        if (i % 7 == 0) {
            vol_cond = !vol_cond;
        }
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", total);
    
    return 0;
}
