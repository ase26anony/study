/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c -o test */
/* Additional flags for LTO: -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Volatile variables to prevent constant propagation */
static volatile int always_true = 1;
static volatile int vol_cond = 1;
static volatile int vol_arg1 = 10;
static volatile int vol_arg2 = 20;
static volatile int vol_arg3 = 30;
static volatile int vol_arg4 = 40;

/* Result accumulator to prevent dead code elimination */
static volatile int global_result = 0;

/* Function to create rematerialization candidates */
static int __attribute__((noinline)) 
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) 
{
    /* Local array for address calculations */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i * 2;
    }
    
    /* Variables to accumulate results */
    volatile int result = 0;
    volatile int temp;
    
    /* Loop to encourage rematerialization analysis */
    for (int iter = 0; iter < 100; iter++) {
        /* BLOCK A: Create rematerialization candidates */
        /* Candidate 1: Simple arithmetic on volatile args (cheap to recompute) */
        int cand1 = arg1 + 5;  /* arg1 + 5 */
        int cand2 = arg2 * 2;  /* arg2 * 2 */
        
        /* Candidate 2: Address calculation with constant offset */
        int *cand3 = &local_array[arg3 + 3];  /* &local_array[arg3 + 3] */
        
        /* Candidate 3: More complex but still recomputable */
        int cand4 = (arg1 * arg4) / 2;
        
        /* Immediate use of candidates in block A */
        temp = cand1 + cand2;
        result += temp;
        result += *cand3;
        result += cand4;
        
        /* Control flow to split live ranges */
        if (vol_cond) {  /* Always true but opaque to compiler */
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int v1 = result + 1;
            int v2 = v1 * 2;
            int v3 = v2 + arg1;
            int v4 = v3 - arg2;
            int v5 = v4 * 3;
            int v6 = v5 / 2;
            int v7 = v6 + arg3;
            int v8 = v7 - arg4;
            int v9 = v8 * 5;
            int v10 = v9 / 3;
            long v11 = v10 + 1000L;
            long v12 = v11 * 2L;
            long v13 = v12 - 500L;
            long v14 = v13 / 2L;
            float f1 = v14 * 1.5f;
            float f2 = f1 + 3.14f;
            float f3 = f2 * 2.0f;
            double d1 = f3 * 1.618;
            double d2 = d1 / 3.14159;
            double d3 = d2 + 2.71828;
            
            /* Additional variables for more pressure */
            int v15 = (int)d3;
            int v16 = v15 + iter;
            int v17 = v16 * 7;
            int v18 = v17 - 11;
            int v19 = v18 / 2;
            int v20 = v19 + 42;
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si vec1 = {v1, v2, v3, v4};
            v4si vec2 = {v5, v6, v7, v8};
            v4si vec3 = vec1 + vec2;
            v4si vec4 = vec3 * (v4si){2, 2, 2, 2};
            int vec_sum = vec4[0] + vec4[1] + vec4[2] + vec4[3];
            result += vec_sum;
            #else
            /* Fallback scalar operations */
            int scalar_sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
            result += scalar_sum;
            #endif
            
            /* Use all temporary variables to prevent elimination */
            result += v9 + v10 + v15 + v16 + v17 + v18 + v19 + v20;
            result += (int)f1 + (int)f2 + (int)f3;
            result += (int)d1 + (int)d2 + (int)d3;
        }
        
        /* BLOCK C: Use candidates again after high pressure region */
        /* This forces compiler to reconsider rematerialization */
        temp = cand1 - cand2;  /* Different use pattern */
        result += temp;
        result += *cand3 * 2;  /* Different offset */
        result += cand4 / 2;
        
        /* Additional use in different context */
        if (always_true) {
            int cand1_alt = arg1 + 5;  /* Same recomputation */
            int cand2_alt = arg2 * 2;
            result += cand1_alt * cand2_alt;
        }
    }
    
    return result;
}

/* Main function with loop to ensure analysis happens */
int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        vol_arg1 = 10 + (i % 5);
        vol_arg2 = 20 + (i % 7);
        vol_arg3 = 30 + (i % 3);
        vol_arg4 = 40 + (i % 4);
        
        int result = test_remat(vol_arg1, vol_arg2, vol_arg3, vol_arg4);
        total += result;
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(total) : : "memory");
    }
    
    global_result = total;
    printf("Result: %d\n", total);
    
    return 0;
}
