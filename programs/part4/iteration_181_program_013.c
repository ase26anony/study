/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink + 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 1.001;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 100000;
    
    /* Arrays to create memory operations */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = i * 2;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 3;
        int v2 = outer + 7;
        int v3 = outer - 5;
        int v4 = outer * 11;
        int v5 = outer / 3;
        int v6 = outer % 17;
        int v7 = outer ^ 0x55;
        int v8 = outer | 0xAA;
        
        double d1 = outer * 0.3;
        double d2 = outer * 0.7;
        double d3 = outer * 1.3;
        double d4 = outer * 2.7;
        double d5 = outer * 0.11;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        accumulator ^= val1;
        
        /* Second use: in conditional with complex control flow */
        if (key_index % 3 == 0) {
            /* Create inner control flow with goto to make analysis non-trivial */
            if (key_index % 2 == 0) {
                double temp = array2[key_index];
                d_accumulator += temp;
                goto inner_block;
            } else {
                int temp = array3[key_index];
                accumulator += temp;
            }
        inner_block:
            /* Third use: after label, with different mode (double) */
            double d_temp = array2[key_index] * 2.0;
            d_accumulator -= d_temp;
        }
        
        /* Fourth use: passed to opaque function */
        int modified = use_int(key_index);
        
        /* Fifth use: in another array index with offset */
        int idx2 = (key_index + 5) % ARRAY_SIZE;
        accumulator += array1[idx2];
        
        /* Sixth use: in switch statement (more control flow) */
        switch (key_index % 4) {
            case 0:
                accumulator ^= v1;
                break;
            case 1:
                /* Use key_index again here */
                accumulator += key_index;
                break;
            case 2:
                accumulator |= v2;
                break;
            case 3:
                /* Another use of key_index */
                accumulator -= key_index;
                break;
        }
        
        /* Use all the pressure variables to keep them live */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        d_accumulator += d1 + d2 + d3 + d4 + d5;
        
        /* Mix in double computations with different modes */
        double d_key = key_index * 0.5;  /* Different mode: DF vs SI */
        d_accumulator = use_double(d_key);
        
        /* Create cyclic data flow with goto */
        if (outer % 100 == 0) {
            goto loop_mid;
        }
        
        continue;
        
    loop_mid:
        /* Use key_index again after goto */
        accumulator += key_index % 10;
        
        /* More register pressure variables */
        int v9 = outer * 13;
        int v10 = outer % 23;
        accumulator += v9 - v10;
    }
    
    /* Final result to prevent complete optimization */
    printf("Result: %d (accumulator), %f (d_accumulator)\n", 
           accumulator, d_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
