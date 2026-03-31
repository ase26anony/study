/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize -o test_remat test_remat.c */
/* For LTO: gcc -O2 -fearly-remat -flto -ffat-lto-objects -fno-omit-frame-pointer -o test_remat test_remat.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg_store;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4)
{
    /* Local variables for register pressure */
    int local1, local2, local3, local4, local5, local6, local7, local8;
    int local9, local10, local11, local12, local13, local14, local15;
    float f1, f2, f3, f4, f5;
    double d1, d2, d3, d4, d5;
    long l1, l2, l3, l4, l5;
    
    /* Local array for address calculations */
    int local_array[100];
    
    /* Result accumulator */
    volatile int result = 0;
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Loop to create multiple uses of candidates */
    for (int iter = 0; iter < 10; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (recomputable) */
        int cand1 = arg1 + 5;  /* arg1 + 5 */
        
        /* Candidate 2: Another arithmetic expression */
        int cand2 = arg2 * 2;  /* arg2 * 2 */
        
        /* Candidate 3: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 3];  /* &local_array[arg3 + 3] */
        
        /* Candidate 4: More complex but still recomputable */
        int cand4 = (arg1 * arg2) + (arg3 - arg4);
        
        /* Immediate use in BLOCK A */
        result += cand1;
        result += cand2;
        result += *cand3;
        result += cand4;
        
        /* Control flow: conditional jump to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            
            /* Many independent arithmetic operations */
            local1 = arg1 + arg2;
            local2 = arg2 + arg3;
            local3 = arg3 + arg4;
            local4 = arg4 + arg1;
            local5 = local1 * local2;
            local6 = local2 * local3;
            local7 = local3 * local4;
            local8 = local4 * local1;
            local9 = local5 + local6;
            local10 = local6 + local7;
            local11 = local7 + local8;
            local12 = local8 + local5;
            local13 = local9 * local10;
            local14 = local10 * local11;
            local15 = local11 * local12;
            
            /* Floating point operations */
            f1 = arg1 * 1.5f;
            f2 = arg2 * 2.5f;
            f3 = arg3 * 3.5f;
            f4 = arg4 * 4.5f;
            f5 = f1 + f2 + f3 + f4;
            
            /* Double operations */
            d1 = arg1 * 1.25;
            d2 = arg2 * 2.25;
            d3 = arg3 * 3.25;
            d4 = arg4 * 4.25;
            d5 = d1 + d2 + d3 + d4;
            
            /* Long operations */
            l1 = arg1 * 100L;
            l2 = arg2 * 200L;
            l3 = arg3 * 300L;
            l4 = arg4 * 400L;
            l5 = l1 + l2 + l3 + l4;
            
#ifdef __SSE2__
            /* Vector operations to increase register pressure */
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1, v2, v3, v4, v5;
            
            /* Initialize vectors */
            int arr1[4] = {arg1, arg2, arg3, arg4};
            int arr2[4] = {arg2, arg3, arg4, arg1};
            int arr3[4] = {arg3, arg4, arg1, arg2};
            
            v1 = *(v4si*)arr1;
            v2 = *(v4si*)arr2;
            v3 = *(v4si*)arr3;
            
            /* Multiple vector operations */
            v4 = v1 + v2;
            v5 = v2 + v3;
            v1 = v4 * v5;
            v2 = v5 - v4;
            v3 = v1 + v2;
            
            /* Use vector results */
            int temp_vec[4];
            *(v4si*)temp_vec = v3;
            result += temp_vec[0];
#endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* More scalar operations after clobber */
            local1 = local1 + local15;
            local2 = local2 + local14;
            local3 = local3 + local13;
            f1 = f1 + f5;
            d1 = d1 + d5;
            l1 = l1 + l5;
            
            /* Use all locals to prevent elimination */
            result += local1 + local2 + local3 + local4 + local5;
            result += local6 + local7 + local8 + local9 + local10;
            result += local11 + local12 + local13 + local14 + local15;
            result += (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
            result += (int)d1 + (int)d2 + (int)d3 + (int)d4;
            result += (int)l1 + (int)l2 + (int)l3 + (int)l4 + (int)l5;
        }
        
        /* BLOCK C: Use candidates again after high-pressure region */
        /* This forces compiler to either rematerialize or replace candidates */
        result += cand1 * 2;
        result += cand2 / 2;
        result += *cand3 + 1;
        result += cand4 - 1;
        
        /* Additional use with different computation */
        int cand1_alt = arg1 + 5;  /* Same as cand1 - recomputable */
        int cand2_alt = arg2 * 2;  /* Same as cand2 - recomputable */
        result += cand1_alt + cand2_alt;
    }
    
    return result;
}

/* Second function with different pattern to increase LTO opportunities */
static volatile int __attribute__((noinline))
test_remat2(volatile int a, volatile int b, volatile int c, volatile int d)
{
    volatile int arr[50];
    volatile int result = 0;
    
    for (int i = 0; i < 50; i++) {
        arr[i] = i;
    }
    
    /* Create candidates */
    int cand1 = a + b + 10;
    int cand2 = &arr[c + 5] - &arr[0];
    int cand3 = (a * b) - (c * d);
    
    /* Use immediately */
    result += cand1 + cand2 + cand3;
    
    /* High pressure block */
    if (always_true) {
        int t1 = a + b;
        int t2 = b + c;
        int t3 = c + d;
        int t4 = d + a;
        int t5 = t1 * t2;
        int t6 = t2 * t3;
        int t7 = t3 * t4;
        int t8 = t4 * t1;
        int t9 = t5 + t6;
        int t10 = t6 + t7;
        
        asm volatile("" ::: "memory");
        
        result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8 + t9 + t10;
    }
    
    /* Use candidates again */
    result += cand1 * 3;
    result += cand2 * 4;
    result += cand3 * 5;
    
    return result;
}

int main(int argc, char **argv)
{
    volatile int iterations = 100;
    volatile int final_result = 0;
    
    /* Use command line argument for iterations if provided */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Call test functions in loop */
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        vol_arg_store = i;
        final_result += test_remat(i, i+1, i+2, i+3);
        final_result += test_remat2(i+1, i+2, i+3, i+4);
    }
    
    /* Print result to prevent elimination */
    printf("Final result: %d\n", final_result);
    
    return 0;
}
