/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg1, vol_arg2, vol_arg3;

/* Function to create rematerialization candidates */
static volatile int test_remat(volatile int a, volatile int b, volatile int c) {
    /* Local variables for register pressure */
    int local1, local2, local3, local4, local5, local6, local7, local8;
    int local9, local10, local11, local12, local13, local14, local15;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Local array for address calculations */
    int local_array[100];
    
    /* Initialize some values */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    volatile int result = 0;
    
    /* Loop to encourage rematerialization analysis */
    for (int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = a + 5;  /* arg + constant */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = b * 2;  /* arg * constant */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[a % 50];  /* &array[arg % constant] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (a << 2) | (b & 0xFF);
        
        /* Use candidates immediately in Block A */
        result += cand1;
        result += *cand3;
        result += cand2 + cand4;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (vol_cond) {
            /* BLOCK B: High register pressure region */
            /* This should cause register pressure and force reconsideration
               of rematerialization decisions */
            
            /* Many independent arithmetic operations */
            local1 = a * b + c;
            local2 = b * c - a;
            local3 = c * a + b;
            local4 = local1 * local2;
            local5 = local2 * local3;
            local6 = local3 * local1;
            local7 = local4 + local5;
            local8 = local5 + local6;
            local9 = local6 + local4;
            local10 = local7 * local8;
            local11 = local8 * local9;
            local12 = local9 * local7;
            local13 = local10 + local11;
            local14 = local11 + local12;
            local15 = local12 + local10;
            
            /* Floating point operations */
            f1 = a * 1.5f;
            f2 = b * 2.5f;
            f3 = c * 3.5f;
            f4 = f1 + f2;
            f5 = f2 + f3;
            f1 = f3 + f4;
            f2 = f4 + f5;
            f3 = f5 + f1;
            
            /* Double precision operations */
            d1 = a * 1.5;
            d2 = b * 2.5;
            d3 = c * 3.5;
            d4 = d1 + d2;
            d5 = d2 + d3;
            d1 = d3 + d4;
            d2 = d4 + d5;
            d3 = d5 + d1;
            
            /* Long operations */
            l1 = a * 100L;
            l2 = b * 200L;
            l3 = c * 300L;
            l4 = l1 + l2;
            l5 = l2 + l3;
            l1 = l3 + l4;
            l2 = l4 + l5;
            l3 = l5 + l1;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1, v2, v3, v4, v5;
            v1 = (v4si){a, b, c, a+b};
            v2 = (v4si){b, c, a, b+c};
            v3 = (v4si){c, a, b, c+a};
            v4 = v1 + v2;
            v5 = v2 + v3;
            v1 = v3 + v4;
            v2 = v4 + v5;
            v3 = v5 + v1;
            #else
            /* Fallback: more scalar operations */
            int extra1 = a * b * c;
            int extra2 = b * c * a;
            int extra3 = c * a * b;
            int extra4 = extra1 + extra2;
            int extra5 = extra2 + extra3;
            extra1 = extra3 + extra4;
            extra2 = extra4 + extra5;
            extra3 = extra5 + extra1;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all these variables to prevent elimination */
            result += local1 + local2 + local3 + local4 + local5;
            result += local6 + local7 + local8 + local9 + local10;
            result += local11 + local12 + local13 + local14 + local15;
            result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4 + (int)d5;
            result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
            
            /* BLOCK C: Use candidates again after high pressure region */
            /* This should trigger filter_old_remats to replace old candidates */
            result += cand1 * 2;      /* Use cand1 again */
            result += cand2 / 2;      /* Use cand2 again */
            result += *cand3 + 10;    /* Use cand3 again */
            result += cand4 << 1;     /* Use cand4 again */
            
            /* More operations to ensure values are live */
            result += (cand1 + cand2) * (cand3 - cand4);
        }
        
        /* Alternate path to create more control flow complexity */
        if (iter % 2 == 0) {
            /* Use candidates in different context */
            result -= cand1;
            result -= cand2;
        }
    }
    
    return result;
}

/* Main function with loop to accumulate results */
int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Set volatile arguments to prevent constant propagation */
    vol_arg1 = argc;
    vol_arg2 = iterations;
    vol_arg3 = argc * 2;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        vol_arg1 = (vol_arg1 * 1103515245 + 12345) & 0x7fffffff;
        vol_arg2 = (vol_arg2 * 1664525 + 1013904223) & 0x7fffffff;
        vol_arg3 = (vol_arg3 * 134775813 + 1) & 0x7fffffff;
        
        /* Call test function */
        total += test_remat(vol_arg1 % 100, vol_arg2 % 100, vol_arg3 % 100);
        
        /* Toggle condition to create varying control flow */
        vol_cond = (i % 3) != 0;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
