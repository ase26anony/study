/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;

/* Function with rematerialization candidates that become "old remats" */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local1, local2, local3, local4, local5, local6, local7, local8;
    int local9, local10, local11, local12, local13, local14, local15;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4;
    long l1, l2, l3, l4, l5;
    
    /* Local array for address calculations */
    int local_array[100];
    memset(local_array, 0, sizeof(local_array));
    
    volatile int result = 0;
    
    /* Loop to create multiple uses of rematerialization candidates */
    for (volatile int i = 0; i < arg4; i++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Simple recomputable expressions - strong rematerialization candidates */
        int cand1 = arg1 + 5;           /* Constant offset from argument */
        int cand2 = arg2 * 2;           /* Simple arithmetic */
        int cand3 = arg3 + i;           /* Loop-invariant + loop counter */
        int *cand4 = &local_array[arg1 + 10];  /* Address calculation with constant offset */
        
        /* Immediate use of candidates in BLOCK A */
        result += cand1;
        result += cand2;
        result += cand3;
        result += *cand4;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true at runtime, opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Dense sequence of independent arithmetic operations */
            local1 = arg1 * arg2;
            local2 = arg2 + arg3;
            local3 = arg3 * arg1;
            local4 = arg1 - arg2;
            local5 = arg2 - arg3;
            local6 = arg3 - arg1;
            local7 = arg1 ^ arg2;
            local8 = arg2 | arg3;
            local9 = arg3 & arg1;
            local10 = arg1 << 2;
            local11 = arg2 >> 1;
            local12 = arg3 * 3;
            local13 = arg1 + 7;
            local14 = arg2 - 5;
            local15 = arg3 + 9;
            
            /* Floating point operations for FP register pressure */
            f1 = arg1 * 1.5f;
            f2 = arg2 * 2.5f;
            f3 = arg3 * 3.5f;
            f4 = f1 + f2;
            f5 = f3 - f1;
            
            /* Double precision operations */
            d1 = (double)arg1 / 2.0;
            d2 = (double)arg2 / 3.0;
            d3 = (double)arg3 / 4.0;
            d4 = d1 * d2 + d3;
            
            /* Long integer operations */
            l1 = (long)arg1 * 1000L;
            l2 = (long)arg2 * 2000L;
            l3 = (long)arg3 * 3000L;
            l4 = l1 + l2;
            l5 = l3 - l1;
            
            /* Vector operations if available (SSE2) */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg1};
            v4si v2 = {arg2, arg3, arg1, arg2};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 - v4;
            
            /* Use vector results to prevent elimination */
            int *vp = (int*)&v5;
            local1 += vp[0];
            local2 += vp[1];
            local3 += vp[2];
            local4 += vp[3];
            #endif
            
            /* Memory clobber to force spills and disrupt register allocation */
            asm volatile("" ::: "memory");
            
            /* More operations to increase pressure */
            local1 = local1 * local2 + local3;
            local4 = local4 - local5 * local6;
            local7 = local7 ^ local8 | local9;
            local10 = local10 << (local11 & 3);
            local12 = local12 * local13 - local14;
            
            f1 = f2 * f3 + f4;
            f5 = f5 - f1 * 0.5f;
            
            d1 = d2 * d3 + d4;
            d2 = d1 / (d3 + 1.0);
            
            l1 = l2 + l3 * l4;
            l5 = l1 - l2 / 100L;
            
            /* Another memory clobber */
            asm volatile("" ::: "memory");
            
            /* BLOCK C: Use rematerialization candidates again */
            /* This forces compiler to either rematerialize or replace old candidates */
            result += cand1 * 2;
            result += cand2 / 2;
            result += cand3 + 1;
            result += *(cand4 + 1);  /* Different offset to prevent CSE */
            
            /* Use all local variables to prevent dead code elimination */
            result += local1 + local2 + local3 + local4 + local5;
            result += local6 + local7 + local8 + local9 + local10;
            result += local11 + local12 + local13 + local14 + local15;
            result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
            result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
        }
        
        /* Additional control flow variation */
        if (i % 2) {
            /* Another use of candidates in different block */
            result -= cand1;
            result -= cand2;
        }
    }
    
    return result;
}

/* Wrapper to create more optimization context */
static volatile int __attribute__((noinline))
remat_wrapper(volatile int a, volatile int b, volatile int c, volatile int d)
{
    int r1 = test_remat(a, b, c, d);
    int r2 = test_remat(b, c, d, a);
    int r3 = test_remat(c, d, a, b);
    int r4 = test_remat(d, a, b, c);
    
    return r1 + r2 + r3 + r4;
}

int main(int argc, char **argv)
{
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    /* Multiple calls with different arguments to create various remat scenarios */
    for (volatile int i = 0; i < iterations; i++) {
        total += remat_wrapper(i, i+1, i+2, 10);
        total += remat_wrapper(i*2, i*3, i*4, 5);
        total += remat_wrapper(i+10, i+20, i+30, 3);
    }
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return 0;
}
