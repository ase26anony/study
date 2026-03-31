/* Compile with: gcc -O2 -fearly-remat -fno-omit-frame-pointer -fno-schedule-insns -fno-tree-vectorize test.c */
/* For LTO testing: gcc -O2 -fearly-remat -flto -ffat-lto-objects test.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Force values to be recomputable but not constant-folded */
static volatile int always_true = 1;
static volatile int arg_force = 0;

/* Vector extensions for register pressure */
#ifdef __SSE2__
typedef int v4si __attribute__((vector_size(16)));
typedef float v4sf __attribute__((vector_size(16)));
#endif

/* Test function with rematerialization candidates */
static volatile int test_remat(volatile int arg1, volatile int arg2, volatile int arg3) {
    volatile int result = 0;
    
    /* Local variables for register pressure */
    int local_array[100];
    for (int i = 0; i < 100; i++) {
        local_array[i] = i + arg_force;
    }
    
    /* Rematerialization candidates - cheap to recompute */
    int cand1 = arg1 + 5;           /* Simple arithmetic */
    int cand2 = arg2 * 2;           /* Another simple computation */
    int *cand3 = &local_array[arg3]; /* Address calculation */
    int cand4 = (arg1 << 2) | arg2; /* Bit operations */
    
    /* Use candidates immediately in block A */
    result += cand1;
    result += cand2;
    result += *cand3;
    result += cand4;
    
    /* Control flow to split live ranges */
    if (always_true) {  /* Block B - always taken but opaque to compiler */
        /* High register pressure region */
        int t1 = arg1 + arg2;
        int t2 = arg2 + arg3;
        int t3 = arg3 + arg1;
        long t4 = (long)arg1 * arg2;
        long t5 = (long)arg2 * arg3;
        long t6 = (long)arg3 * arg1;
        float f1 = (float)arg1 / 3.0f;
        float f2 = (float)arg2 / 7.0f;
        float f3 = (float)arg3 / 11.0f;
        double d1 = (double)arg1 * 1.234;
        double d2 = (double)arg2 * 5.678;
        double d3 = (double)arg3 * 9.012;
        
        /* More variables to increase pressure */
        int t7 = t1 + t2;
        int t8 = t2 + t3;
        int t9 = t3 + t1;
        long t10 = t4 + t5;
        long t11 = t5 + t6;
        long t12 = t6 + t4;
        float f4 = f1 + f2;
        float f5 = f2 + f3;
        float f6 = f3 + f1;
        double d4 = d1 + d2;
        double d5 = d2 + d3;
        double d6 = d3 + d1;
        
        /* Vector operations if available */
#ifdef __SSE2__
        v4si v1 = {arg1, arg2, arg3, arg1 + arg2};
        v4si v2 = {arg2, arg3, arg1, arg2 + arg3};
        v4si v3 = v1 + v2;
        v4si v4 = v1 * v2;
        v4si v5 = v3 + v4;
        
        v4sf vf1 = {(float)arg1, (float)arg2, (float)arg3, (float)arg1};
        v4sf vf2 = vf1 * (v4sf){2.0f, 3.0f, 4.0f, 5.0f};
        v4sf vf3 = vf1 + vf2;
#endif
        
        /* Memory clobber to force spills */
        asm volatile("" ::: "memory");
        
        /* Use all temporaries to prevent DCE */
        result += t7 + t8 + t9;
        result += (int)(t10 + t11 + t12);
        result += (int)(f4 + f5 + f6);
        result += (int)(d4 + d5 + d6);
#ifdef __SSE2__
        result += v3[0] + v4[1] + v5[2];
        result += (int)vf3[0];
#endif
    }
    
    /* Block C - Use candidates again after high pressure region */
    result += cand1 * 2;
    result += cand2 / 2;
    result += *cand3 + 1;
    result += cand4 ^ 0xFF;
    
    /* Additional computations to ensure values are live across blocks */
    int final1 = cand1 + cand2;
    int final2 = *cand3 - cand4;
    result += final1 + final2;
    
    return result;
}

/* Main function with loop to create optimization context */
int main(int argc, char **argv) {
    volatile int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    volatile int total = 0;
    
    /* Create different argument patterns */
    volatile int arg_base = 42;
    
    for (volatile int i = 0; i < iterations; i++) {
        /* Vary arguments slightly each iteration */
        volatile int arg1 = arg_base + (i % 10);
        volatile int arg2 = arg_base * 2 + (i % 7);
        volatile int arg3 = arg_base / 2 + (i % 5);
        
        /* Call test function in loop */
        total += test_remat(arg1, arg2, arg3);
        
        /* Modify global to prevent optimization */
        arg_force = i;
    }
    
    printf("Result: %d\n", total);
    
    /* Additional test with different control flow */
    volatile int alt_total = 0;
    for (volatile int j = 0; j < 100; j++) {
        /* Nested loops create more complex CFG */
        for (volatile int k = 0; k < 10; k++) {
            alt_total += test_remat(j, k, j + k);
        }
    }
    
    printf("Alt result: %d\n", alt_total);
    
    return total > alt_total ? 0 : 1;
}
