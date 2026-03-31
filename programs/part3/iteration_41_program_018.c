/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_arg4 = 40;

/* Function to create rematerialization candidates and trigger filter_old_remats */
static volatile int test_remat(volatile int a, volatile int b, volatile int c, volatile int d)
{
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Many local variables to increase register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    double d1, d2, d3, d4, d5, d6, d7, d8, d9, d10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (int iter = 0; iter < 100; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        /* Candidate 1: Constant derived from volatile argument (cheap to recompute) */
        int cand1 = a + 5;  /* a + 5 is recomputable */
        
        /* Candidate 2: Another constant expression */
        int cand2 = b * 2;  /* b * 2 is recomputable */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[c];  /* &local_array[c] is recomputable */
        
        /* Candidate 4: More complex but still recomputable expression */
        int cand4 = (a + b) * 3 - d;
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand2;
        result += cand4;
        
        /* Conditional jump based on volatile to split control flow */
        /* This creates separate basic blocks for the live ranges */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* Inline assembly with memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense sequence of independent arithmetic operations */
            v1 = a * b + c;
            v2 = b * c + d;
            v3 = c * d + a;
            v4 = d * a + b;
            v5 = v1 + v2 + v3 + v4;
            v6 = v5 * 2 - v1;
            v7 = v6 / 3 + v2;
            v8 = v7 << 2;
            v9 = v8 >> 1;
            v10 = v9 ^ v3;
            v11 = v10 | v4;
            v12 = v11 & v5;
            v13 = v12 + v6;
            v14 = v13 - v7;
            v15 = v14 * v8;
            v16 = v15 / (v9 + 1);
            v17 = v16 << v10;
            v18 = v17 >> 1;
            v19 = v18 ^ v11;
            v20 = v19 | v12;
            
            /* Floating point operations to consume FP registers */
            f1 = a * 1.1f;
            f2 = b * 2.2f;
            f3 = c * 3.3f;
            f4 = d * 4.4f;
            f5 = f1 + f2;
            f6 = f3 - f4;
            f7 = f5 * f6;
            f8 = f7 / 2.0f;
            f9 = f8 + f1;
            f10 = f9 - f2;
            
            /* Double precision operations */
            d1 = a * 1.111;
            d2 = b * 2.222;
            d3 = c * 3.333;
            d4 = d * 4.444;
            d5 = d1 + d2;
            d6 = d3 - d4;
            d7 = d5 * d6;
            d8 = d7 / 2.0;
            d9 = d8 + d1;
            d10 = d9 - d2;
            
            /* Long integer operations */
            l1 = (long)a * 1000L;
            l2 = (long)b * 2000L;
            l3 = (long)c * 3000L;
            l4 = (long)d * 4000L;
            l5 = l1 + l2;
            l6 = l3 - l4;
            l7 = l5 * l6;
            l8 = l7 / 100L;
            l9 = l8 + l1;
            l10 = l9 - l2;
            
#ifdef __SSE2__
            /* Vector operations to consume vector registers if available */
            typedef int v4si __attribute__((vector_size(16)));
            v4si vec1 = {a, b, c, d};
            v4si vec2 = {b, c, d, a};
            v4si vec3 = {c, d, a, b};
            v4si vec4 = vec1 + vec2;
            v4si vec5 = vec3 - vec1;
            v4si vec6 = vec4 * vec5;
            v4si vec7 = vec6 + vec2;
            v4si vec8 = vec7 - vec3;
            
            /* Use vector results in scalar computation */
            int vec_sum = vec8[0] + vec8[1] + vec8[2] + vec8[3];
            result += vec_sum;
#endif
            
            /* Another memory clobber to ensure register pressure */
            asm volatile("" ::: "memory");
            
            /* Use all temporary variables to prevent elimination */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
            result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
            result += (int)f6 + (int)f7 + (int)f8 + (int)f9 + (int)f10;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
            result += (int)d6 + (int)d7 + (int)d8 + (int)d9 + (int)d10;
            result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
            result += (int)l6 + (int)l7 + (int)l8 + (int)l9 + (int)l10;
        }
        
        /* BLOCK C: Use rematerialization candidates again after high-pressure region */
        /* This forces the compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;
        result += cand2 + 10;
        result += *cand3 * 3;
        result += cand4 - 5;
        
        /* Additional use to ensure candidates are live across blocks */
        if (iter % 2 == 0) {
            result += cand1 + cand2;
        } else {
            result += *cand3 + cand4;
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
    
    volatile int total_result = 0;
    
    /* Call test_remat multiple times to give compiler more optimization context */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly to prevent complete optimization */
        vol_arg1 = 10 + (i % 5);
        vol_arg2 = 20 + (i % 7);
        vol_arg3 = 30 + (i % 11);
        vol_arg4 = 40 + (i % 13);
        
        total_result += test_remat(vol_arg1, vol_arg2, vol_arg3, vol_arg4);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total_result);
    
    return 0;
}
