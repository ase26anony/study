/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent constant propagation */
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_iter = 1000;

/* Vector extensions for register pressure */
#ifdef __SSE2__
#include <xmmintrin.h>
typedef int v4si __attribute__((vector_size(16)));
#endif

/* Function with rematerialization candidates */
static volatile int test_remat(volatile int a, volatile int b, volatile int c) {
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Volatile result to prevent elimination */
    volatile int result = 0;
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < vol_iter; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = a + 5;  /* a + 5 is cheap to recompute */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[b + 2];  /* &local_array[b+2] is recomputable */
        
        /* Candidate 3: Another arithmetic expression */
        int cand3 = c * 2 - 3;
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += *cand2;
        result += cand3;
        
        /* Control flow to split live ranges */
        /* Condition always true at runtime but opaque to compiler */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* Many distinct local variables to consume registers */
            int t1 = result + 1;
            int t2 = t1 * 2;
            int t3 = t2 - a;
            int t4 = t3 + b;
            int t5 = t4 * c;
            long t6 = t5 * 3L;
            long t7 = t6 + 1000L;
            long t8 = t7 - t5;
            float f1 = t8 * 0.5f;
            float f2 = f1 + 1.5f;
            float f3 = f2 * 2.0f;
            double d1 = f3 * 1.25;
            double d2 = d1 + 3.14159;
            double d3 = d2 * 2.0;
            int t9 = d3;
            int t10 = t9 + t1;
            int t11 = t10 * t2;
            int t12 = t11 - t3;
            int t13 = t12 + t4;
            int t14 = t13 * t5;
            int t15 = t14 / 7;
            
            /* Vector operations for additional register pressure */
            #ifdef __SSE2__
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t7, t8};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int *vp = (int*)&v5;
            t15 += vp[0] + vp[1] + vp[2] + vp[3];
            #else
            /* Fallback scalar operations */
            t15 += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use many variables to keep them live */
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10 +
                     t11 + t12 + t13 + t14 + t15 + f1 + f2 + f3 + d1 + d2 + d3;
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces compiler to either rematerialize or replace old candidates */
        result += cand1 * 2;
        result += *cand2 + 1;
        result += cand3 - 1;
        
        /* Additional computations to create more register pressure */
        int extra1 = cand1 + cand3;
        int extra2 = *cand2 - cand1;
        result += extra1 * extra2;
    }
    
    return result;
}

/* Second function with different patterns */
static volatile int test_remat2(volatile int x, volatile int y) {
    volatile int arr[50];
    for (int i = 0; i < 50; i++) {
        arr[i] = i + x;
    }
    
    volatile int sum = 0;
    
    /* Multiple candidate values */
    for (volatile int j = 0; j < 100; j++) {
        /* Candidates with different expression types */
        int cand4 = x + y + j;          /* Multiple args */
        int *cand5 = &arr[y + 3];       /* Address with offset */
        int cand6 = (x << 2) | (y & 0xF); /* Bit operations */
        
        /* Use candidates */
        sum += cand4;
        sum += *cand5;
        sum += cand6;
        
        if (vol_cond) {
            /* High register pressure block */
            int r1 = sum + x;
            int r2 = r1 * y;
            int r3 = r2 - j;
            int r4 = r3 + cand4;
            int r5 = r4 * 2;
            long r6 = r5 * 5L;
            long r7 = r6 + 100L;
            float r8 = r7 * 0.25f;
            double r9 = r8 * 1.1;
            
            /* More variables */
            int r10 = r9;
            int r11 = r10 + r1;
            int r12 = r11 * r2;
            int r13 = r12 - r3;
            int r14 = r13 + r4;
            int r15 = r14 * r5;
            
            /* Memory barrier */
            asm volatile("" ::: "memory");
            
            sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 +
                   r11 + r12 + r13 + r14 + r15;
        }
        
        /* Reuse candidates */
        sum += cand4 * 3;
        sum += *cand5 - 2;
        sum += cand6 + 1;
    }
    
    return sum;
}

int main(int argc, char **argv) {
    /* Use command line argument for iteration count if provided */
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    vol_iter = iterations;
    
    volatile int total = 0;
    
    /* Call test functions multiple times to give compiler more context */
    for (int i = 0; i < 10; i++) {
        total += test_remat(vol_arg1 + i, vol_arg2 + i, vol_arg3 + i);
        total += test_remat2(vol_arg1 - i, vol_arg2 + i * 2);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
