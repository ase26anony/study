/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int global_seed = 42;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local_array[64];
    volatile int result = 0;
    
    /* Initialize array with volatile pattern */
    for (int i = 0; i < 64; i++) {
        local_array[i] = global_seed + i;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < arg4; iter++) {
        /* --- BLOCK A: Create rematerialization candidates --- */
        /* Candidate 1: Simple arithmetic on volatile argument */
        int cand1 = arg1 + 10;  /* arg1 + constant */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand2 = &local_array[arg2 + 5];  /* &array[arg2 + 5] */
        
        /* Candidate 3: More complex but still recomputable */
        int cand3 = (arg1 * 2) + (arg3 >> 1);
        
        /* Immediate use of candidates (creates initial remat opportunity) */
        result += *cand2;
        result += cand1;
        result += cand3;
        
        /* --- Control flow to split live ranges --- */
        if (always_true) {  /* Always taken, but opaque to compiler */
            /* --- BLOCK B: High register pressure region --- */
            /* Many independent variables to consume registers */
            int t1 = result * 2;
            int t2 = t1 + arg1;
            int t3 = t2 - arg2;
            int t4 = t3 * arg3;
            int t5 = t4 >> 2;
            int t6 = t5 & 0xFF;
            int t7 = t6 | 0x80;
            int t8 = t7 ^ arg4;
            int t9 = t8 << 1;
            int t10 = t9 + 100;
            long t11 = t10 * 3L;
            long t12 = t11 - 50L;
            long t13 = t12 / 2L;
            long t14 = t13 | 0xFFFF;
            long t15 = t14 & 0xFFFFFF;
            
            /* Floating point variables for FP register pressure */
            float f1 = t1 * 0.5f;
            float f2 = f1 + 1.0f;
            float f3 = f2 * 2.0f;
            float f4 = f3 - 0.5f;
            float f5 = f4 / 3.0f;
            double d1 = t11 * 0.25;
            double d2 = d1 + 1.5;
            double d3 = d2 * 2.5;
            double d4 = d3 - 0.75;
            double d5 = d4 / 1.25;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {t1, t2, t3, t4};
            v4si v2 = {t5, t6, t7, t8};
            v4si v3 = v1 + v2;
            v4si v4 = v3 * v1;
            v4si v5 = v4 - v2;
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback: more scalar operations */
            int extra1 = t1 * t2;
            int extra2 = t3 * t4;
            int extra3 = t5 * t6;
            int extra4 = t7 * t8;
            int extra5 = t9 * t10;
            result += extra1 + extra2 + extra3 + extra4 + extra5;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to keep them live */
            result += t15;
            result += (int)f5;
            result += (int)d5;
            
            /* --- BLOCK C: Use candidates again after high pressure --- */
            /* This forces reconsideration of rematerialization */
            result += cand1 * 2;      /* Use cand1 again */
            result += *cand2 + 5;     /* Use cand2 again */
            result += cand3 >> 1;     /* Use cand3 again */
            
            /* More operations to prevent dead code elimination */
            result += local_array[iter % 64];
        } else {
            /* Unreachable but needed for control flow */
            result += arg1 + arg2;
        }
    }
    
    return result;
}

/* Second function with different patterns */
static volatile int __attribute__((noinline))
test_remat2(volatile int a, volatile int b, volatile int c)
{
    volatile int arr[32];
    volatile int sum = 0;
    
    /* Initialize */
    for (int i = 0; i < 32; i++) {
        arr[i] = i * 2 + global_seed;
    }
    
    /* Multiple candidate computations in loop */
    for (volatile int i = 0; i < c; i++) {
        /* Candidates with different expression types */
        int cand4 = a * 3 + b;
        int cand5 = (b << 2) | 0xF;
        int *cand6 = &arr[(a + i) % 32];
        
        /* Initial uses */
        sum += cand4;
        sum += cand5;
        sum += *cand6;
        
        if (always_true) {
            /* High register pressure with mixed types */
            long l1 = sum * 100L;
            long l2 = l1 - 50L;
            long l3 = l2 / 3L;
            float f1 = l3 * 0.1f;
            float f2 = f1 + 2.0f;
            double d1 = l2 * 0.01;
            double d2 = d1 - 1.0;
            
            /* Many integer operations */
            int x1 = a + b;
            int x2 = x1 * c;
            int x3 = x2 >> 1;
            int x4 = x3 & 0xFF;
            int x5 = x4 | 0x80;
            int x6 = x5 ^ i;
            int x7 = x6 << 2;
            int x8 = x7 + 64;
            int x9 = x8 - 32;
            int x10 = x9 * 2;
            int x11 = x10 / 3;
            int x12 = x11 % 100;
            int x13 = x12 + x1;
            int x14 = x13 * x2;
            int x15 = x14 >> 2;
            
            /* Memory clobber */
            asm volatile("" ::: "memory");
            
            /* Use all variables */
            sum += l3 + (int)f2 + (int)d2;
            sum += x15;
            
            /* Reuse candidates */
            sum += cand4 - cand5;
            sum += *cand6 * 2;
        }
    }
    
    return sum;
}

int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Call test functions multiple times with different arguments */
    for (int i = 0; i < iterations; i++) {
        total += test_remat(
            global_seed + i,
            (i * 3) % 64,
            (i * 5) % 32,
            5  /* Small loop count */
        );
        
        total += test_remat2(
            global_seed - i,
            (i * 7) % 16,
            3  /* Small loop count */
        );
        
        /* Modify global seed to change patterns */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", total);
    
    /* Also test with LTO-friendly compilation */
    if (argc > 2) {
        /* Additional test case */
        volatile int extra = test_remat(1, 2, 3, 4) + test_remat2(4, 5, 6);
        printf("Extra: %d\n", extra);
    }
    
    return 0;
}
