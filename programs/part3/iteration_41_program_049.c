/* Compile with: gcc -O2 -fearly-remat -fdump-rtl-early-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force recomputable expressions that can't be constant folded */
static volatile int global_seed = 12345;

/* Function to create rematerialization candidates */
static volatile int __attribute__((noinline))
test_remat(volatile int arg1, volatile int arg2, volatile int arg3, volatile int arg4) {
    volatile int result = 0;
    int local_array[256];
    
    /* Initialize local array with volatile pattern */
    for (int i = 0; i < 256; i++) {
        local_array[i] = global_seed + i;
    }
    
    /* Loop to create multiple uses of candidates */
    for (volatile int iter = 0; iter < 3; iter++) {
        /* BLOCK A: Create rematerialization candidates with simple recomputable values */
        int cand1 = arg1 + 10;              /* Simple arithmetic - strong candidate */
        int cand2 = arg2 * 2;               /* Another simple recomputable value */
        int cand3 = arg3 & 0xFF;            /* Mask operation - cheap to recompute */
        
        /* Address calculations - good remat candidates */
        int *addr1 = &local_array[arg1 % 256];
        int *addr2 = &local_array[arg2 % 256 + 5];
        
        /* Immediate use in BLOCK A */
        result += *addr1 + cand1;
        result += *addr2 + cand2;
        result += cand3;
        
        /* Control flow to split live ranges */
        /* Use volatile condition to prevent optimization */
        if (global_seed != 0) {  /* Always true at runtime */
            /* BLOCK B: High register pressure region */
            /* Many independent variables to consume registers */
            int t1 = arg1 * arg2;
            int t2 = arg2 * arg3;
            int t3 = arg3 * arg4;
            int t4 = arg4 * arg1;
            long t5 = (long)arg1 * arg2 * 3;
            long t6 = (long)arg2 * arg3 * 5;
            long t7 = (long)arg3 * arg4 * 7;
            long t8 = (long)arg4 * arg1 * 11;
            float f1 = (float)arg1 * 1.1f;
            float f2 = (float)arg2 * 1.2f;
            float f3 = (float)arg3 * 1.3f;
            float f4 = (float)arg4 * 1.4f;
            double d1 = (double)arg1 * 1.01;
            double d2 = (double)arg2 * 1.02;
            double d3 = (double)arg3 * 1.03;
            double d4 = (double)arg4 * 1.04;
            
            /* More variables to increase pressure */
            int t9 = t1 + t2;
            int t10 = t3 + t4;
            long t11 = t5 + t6;
            long t12 = t7 + t8;
            float f5 = f1 + f2;
            float f6 = f3 + f4;
            double d5 = d1 + d2;
            double d6 = d3 + d4;
            
            /* Vector operations if available */
            #ifdef __SSE2__
            typedef int v4si __attribute__((vector_size(16)));
            v4si v1 = {arg1, arg2, arg3, arg4};
            v4si v2 = {arg2, arg3, arg4, arg1};
            v4si v3 = v1 + v2;
            v4si v4 = v1 * v2;
            v4si v5 = v3 + v4;
            
            /* Use vector results */
            int vsum = v5[0] + v5[1] + v5[2] + v5[3];
            result += vsum;
            #else
            /* Fallback scalar operations */
            int vsum = (arg1 + arg2) + (arg2 + arg3) + (arg3 + arg4) + (arg4 + arg1);
            result += vsum;
            #endif
            
            /* Memory clobber to force spills */
            asm volatile("" ::: "memory");
            
            /* Use all temporaries to prevent elimination */
            result += t9 + t10;
            result += (int)(t11 % 1000) + (int)(t12 % 1000);
            result += (int)f5 + (int)f6;
            result += (int)d5 + (int)d6;
        }
        
        /* BLOCK C: Use candidate values again after high pressure region */
        /* This requires rematerialization or replacement */
        result += cand1 * 2;
        result += cand2 / 2;
        result += cand3 | 0x80;
        result += *addr1 - *addr2;
        
        /* More computations to extend live ranges */
        int cand4 = arg4 + 20;  /* New candidate in same block */
        int cand5 = (arg1 + arg2) * 3;
        
        result += cand4 + cand5;
        
        /* Another high pressure spike */
        if (global_seed > 0) {
            int p1 = arg1 * 3, p2 = arg2 * 4, p3 = arg3 * 5, p4 = arg4 * 6;
            int p5 = p1 + p2, p6 = p3 + p4, p7 = p5 * p6;
            float p8 = (float)p7 * 0.5f;
            double p9 = (double)p7 * 0.25;
            result += (int)p8 + (int)p9;
            asm volatile("" ::: "memory");
        }
        
        /* Final use of candidates */
        result += (cand1 + cand2 + cand3) % 100;
    }
    
    return result;
}

/* Secondary function to create cross-function optimization opportunities */
static volatile int __attribute__((noinline))
helper_func(volatile int x, volatile int y) {
    /* Create more rematerialization candidates */
    int cand_a = x + y * 2;
    int cand_b = (x & 0xF) | (y << 4);
    
    /* High register pressure in helper */
    int t1 = x * 3, t2 = y * 4, t3 = x * y;
    int t4 = t1 + t2, t5 = t2 + t3, t6 = t1 + t3;
    float f1 = (float)t4 * 1.5f;
    double d1 = (double)t5 * 2.5;
    
    asm volatile("" ::: "memory");
    
    return cand_a + cand_b + t6 + (int)f1 + (int)d1;
}

int main(int argc, char **argv) {
    volatile int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    volatile int final_result = 0;
    
    /* Create varying inputs to prevent constant propagation */
    volatile int base = global_seed;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        volatile int arg1 = base + i;
        volatile int arg2 = base + i * 2;
        volatile int arg3 = base + i * 3;
        volatile int arg4 = base + i * 4;
        
        /* Call test function */
        int r1 = test_remat(arg1, arg2, arg3, arg4);
        
        /* Call helper to create LTO opportunities */
        int r2 = helper_func(arg1, arg2);
        
        final_result += r1 + r2;
        
        /* Modify global seed to change control flow */
        global_seed = (global_seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    /* Print result to prevent elimination */
    printf("Result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
