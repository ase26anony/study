/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat_lto test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_iter = 100;

/* Vector types for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int a, volatile int b, volatile int c) {
    volatile int result = 0;
    
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* High register pressure variables */
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    long l1, l2, l3, l4, l5;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < vol_iter; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = a + 5;  /* a + 5 is recomputable */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[b];  /* &local_array[b] is recomputable */
        
        /* Candidate 3: More complex but still recomputable expression */
        int cand3 = (a * 2) + (b / 2) - c;
        
        /* Immediate use of candidates in Block A */
        result += cand1;
        result += *cand2;
        result += cand3;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            
            /* Many independent arithmetic operations */
            r1 = a * b + c;
            r2 = b * c + a;
            r3 = c * a + b;
            r4 = r1 * r2 + r3;
            r5 = r2 * r3 + r1;
            r6 = r3 * r1 + r2;
            r7 = r4 * r5 + r6;
            r8 = r5 * r6 + r4;
            r9 = r6 * r4 + r5;
            r10 = r7 * r8 + r9;
            
            l1 = (long)r1 * r2;
            l2 = (long)r2 * r3;
            l3 = (long)r3 * r4;
            l4 = (long)r4 * r5;
            l5 = (long)r5 * r6;
            
            f1 = (float)r1 / 3.14f;
            f2 = (float)r2 / 2.71f;
            f3 = (float)r3 / 1.41f;
            f4 = (float)r4 / 1.73f;
            f5 = (float)r5 / 0.577f;
            
            d1 = (double)l1 / 3.14159;
            d2 = (double)l2 / 2.71828;
            d3 = (double)l3 / 1.41421;
            d4 = (double)l4 / 1.73205;
            d5 = (double)l5 / 0.577215;
            
            /* Vector operations to consume more registers */
            #ifdef __SSE2__
            v4si vec1 = {r1, r2, r3, r4};
            v4si vec2 = {r5, r6, r7, r8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec1 * vec2;
            v4si vec5 = vec3 - vec4;
            
            v4sf fvec1 = {f1, f2, f3, f4};
            v4sf fvec2 = {f5, f1, f2, f3};
            v4sf fvec3 = fvec1 + fvec2;
            v4sf fvec4 = fvec1 * fvec2;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            r1 = r1 + r10;
            r2 = r2 + r9;
            r3 = r3 + r8;
            r4 = r4 + r7;
            r5 = r5 + r6;
            
            l1 = l1 + l5;
            l2 = l2 + l4;
            l3 = l3 + l3;
            
            f1 = f1 + f5;
            f2 = f2 + f4;
            f3 = f3 + f3;
            
            d1 = d1 + d5;
            d2 = d2 + d4;
            d3 = d3 + d3;
            
            /* BLOCK C: Use candidates again after high pressure */
            /* This forces compiler to either rematerialize or replace */
            result += cand1 * 2;
            result += *cand2 + 1;
            result += cand3 - 1;
            
            /* Use all high-pressure variables to prevent elimination */
            result += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
            result += (int)(l1 + l2 + l3 + l4 + l5);
            result += (int)(f1 + f2 + f3 + f4 + f5);
            result += (int)(d1 + d2 + d3 + d4 + d5);
            
            #ifdef __SSE2__
            /* Use vector results */
            int vec_sum = vec3[0] + vec3[1] + vec3[2] + vec3[3];
            vec_sum += vec4[0] + vec4[1] + vec4[2] + vec4[3];
            vec_sum += vec5[0] + vec5[1] + vec5[2] + vec5[3];
            result += vec_sum;
            #endif
        }
        
        /* Additional candidate with different live range */
        int cand4 = a * b - c;
        if (iter % 2) {
            result += cand4;
        } else {
            result -= cand4;
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
    
    /* Call test function multiple times with different args */
    for (int i = 0; i < iterations; i++) {
        vol_arg1 = 10 + (i % 5);
        vol_arg2 = 20 + (i % 7);
        vol_arg3 = 30 + (i % 11);
        vol_iter = 50 + (i % 20);
        
        total += test_remat(vol_arg1, vol_arg2, vol_arg3);
        
        /* Modify volatile condition occasionally */
        if (i % 3 == 0) {
            vol_cond = !vol_cond;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
