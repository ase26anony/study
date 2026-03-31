/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -o test test.c */
/* Additional flags for testing: -O3 -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int loop_counter = 1000;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, 
                               volatile int arg3, volatile int arg4) {
    /* Local array for address calculations */
    int local_array[256];
    for (int i = 0; i < 256; i++) {
        local_array[i] = i * 3;
    }
    
    /* Many local variables for register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    
    /* Volatile result to prevent optimization */
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < loop_counter; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = arg1 + 5;  /* arg1 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;  /* arg2 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 10];  /* &local_array[arg3 + 10] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg2) + (arg3 - arg4);
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand3;
        result += cand2 + cand4;
        
        /* Conditional jump based on volatile to split control flow */
        if (always_true) {
            /* BLOCK B: High register pressure region */
            
            /* Inline assembly with memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Dense sequence of independent arithmetic operations */
            v1 = arg1 + arg2;
            v2 = arg2 + arg3;
            v3 = arg3 + arg4;
            v4 = arg4 + arg1;
            v5 = v1 * v2;
            v6 = v2 * v3;
            v7 = v3 * v4;
            v8 = v4 * v1;
            v9 = v5 + v6;
            v10 = v7 + v8;
            
            l1 = (long)v1 * v2;
            l2 = (long)v2 * v3;
            l3 = (long)v3 * v4;
            l4 = (long)v4 * v1;
            l5 = l1 + l2 + l3 + l4;
            
            f1 = (float)v1 / 3.14f;
            f2 = (float)v2 / 2.71f;
            f3 = (float)v3 / 1.41f;
            f4 = (float)v4 / 1.73f;
            f5 = f1 + f2 + f3 + f4;
            
            d1 = (double)l1 / 3.14159;
            d2 = (double)l2 / 2.71828;
            d3 = (double)l3 / 1.41421;
            d4 = (double)l4 / 1.73205;
            d5 = d1 + d2 + d3 + d4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            v4si vec1 = {v1, v2, v3, v4};
            v4si vec2 = {v5, v6, v7, v8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 + vec4;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f1 * 2, f2 * 2, f3 * 2, f4 * 2};
            v4sf fvec3 = fvec1 + fvec2;
            
            /* Use vector results */
            int *vp = (int*)&vec5;
            result += vp[0] + vp[1] + vp[2] + vp[3];
            #else
            /* Fallback: more scalar operations */
            int extra1 = v1 * 3 + v2 * 5;
            int extra2 = v3 * 7 + v4 * 11;
            int extra3 = v5 * 13 + v6 * 17;
            int extra4 = v7 * 19 + v8 * 23;
            result += extra1 + extra2 + extra3 + extra4;
            #endif
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            v1 = v1 ^ v2 ^ v3 ^ v4;
            v2 = v5 | v6 | v7 | v8;
            v3 = v9 & v10 & arg1 & arg2;
            v4 = (v1 << 3) | (v2 >> 2);
            v5 = (v3 * 31) % 17;
            
            f1 = f1 * 1.1f + f2 * 1.2f;
            f2 = f3 * 1.3f + f4 * 1.4f;
            f3 = f5 * 1.5f + f1 * 1.6f;
            
            d1 = d1 * 1.01 + d2 * 1.02;
            d2 = d3 * 1.03 + d4 * 1.04;
            d3 = d5 * 1.05 + d1 * 1.06;
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This forces reconsideration of rematerialization */
            result += cand1 * 2;          /* Use cand1 again */
            result += cand2 + arg3;       /* Use cand2 again */
            result += *cand3 + 5;         /* Use cand3 again */
            result += cand4 - arg4;       /* Use cand4 again */
            
            /* Use all the high-pressure variables to keep them live */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            result += (int)l5;
            result += (int)f5;
            result += (int)d5;
        }
        
        /* Alternate path to create more control flow complexity */
        if (iter % 2 == 0) {
            /* Another use of candidates in different context */
            result -= cand1;
            result += cand2 * 3;
        }
    }
    
    return result;
}

/* Wrapper to create more optimization context */
static volatile int __attribute__((noinline)) 
remat_wrapper(volatile int a, volatile int b, volatile int c, volatile int d) {
    volatile int r1 = test_remat(a, b, c, d);
    volatile int r2 = test_remat(b, c, d, a);
    volatile int r3 = test_remat(c, d, a, b);
    volatile int r4 = test_remat(d, a, b, c);
    return r1 + r2 + r3 + r4;
}

int main(int argc, char **argv) {
    /* Use command line argument for loop count */
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    loop_counter = iterations / 10;
    if (loop_counter < 10) loop_counter = 10;
    
    volatile int total = 0;
    
    /* Multiple calls with different arguments */
    for (int i = 0; i < iterations; i++) {
        total += remat_wrapper(i, i+1, i+2, i+3);
        total += remat_wrapper(i*2, i*3, i*4, i*5);
        
        /* Vary the always_true condition occasionally */
        if (i % 7 == 0) {
            always_true = 0;
        } else {
            always_true = 1;
        }
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different patterns */
    volatile int final = 0;
    for (int i = 0; i < 50; i++) {
        final += test_remat(i, i*11, i*13, i*17);
        final += test_remat(i*19, i*23, i*29, i*31);
    }
    
    printf("Final: %d\n", final + total);
    
    return 0;
}
