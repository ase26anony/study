/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o early-remat-trigger
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o early-remat-trigger
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 1000000

int main(void) {
    /* Large arrays to create memory pressure */
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
    
    /* Main loop with high register pressure */
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = i * 3;
        int v2 = i + 17;
        int v3 = i / 5;
        int v4 = i % 13;
        int v5 = i * i;
        int v6 = i | 0xFF;
        int v7 = i & 0x7F;
        int v8 = i ^ 0x55;
        int v9 = i << 2;
        int v10 = i >> 1;
        
        double d1 = i * 0.3;
        double d2 = i + 2.5;
        double d3 = i / 7.0;
        double d4 = i * 0.125;
        double d5 = i * i * 0.01;
        double d6 = d1 + d2;
        double d7 = d3 * d4;
        double d8 = d5 - d6;
        double d9 = d7 / 8.0;
        double d10 = d8 + d9;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - candidate for rematerialization */
        /* This is the key expression that should trigger rematerialization */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Second use: in conditional */
        if (key_index % 3 == 0) {
            v1 += array3[key_index];
        } else {
            v2 -= array1[key_index];
        }
        
        /* Third use: arithmetic operation */
        int val2 = key_index * key_index / 17;
        
        /* Fourth use: array indexing with different array */
        double val3 = array2[key_index];
        
        /* Fifth use: passed to opaque function */
        use_int(key_index);
        
        /* Complex control flow with inner conditional */
        switch (key_index % 5) {
            case 0:
                /* Use key_index again here */
                v3 += key_index;
                use_double(d1 + key_index);
                break;
            case 1:
                v4 -= key_index;
                /* Create another use in a different basic block */
                if (key_index < ARRAY_SIZE / 2) {
                    v5 = array1[key_index] + key_index;
                }
                break;
            case 2:
                /* More uses with different mode (double) */
                d1 += key_index;
                use_double(d1);
                break;
            case 3:
                /* Pointer arithmetic use */
                use_ptr(&array1[key_index]);
                v6 = key_index * 3;
                break;
            case 4:
                /* Nested conditional with key_index use */
                if (key_index % 7 == 0) {
                    v7 = key_index / 2;
                } else {
                    v8 = key_index * 2;
                }
                /* Use in volatile operation */
                volatile int tmp = key_index;
                v9 += tmp;
                break;
        }
        
        /* Use all variables to prevent dead code elimination */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        accumulator ^= (int)d1 ^ (int)d2 ^ (int)d3;
        
        /* Use volatile sink for recomputed values */
        volatile int sink1 = val1;
        volatile int sink2 = val2;
        volatile double sink3 = val3;
        
        (void)sink1; (void)sink2; (void)sink3;
        
        /* Small inner loop to create cyclic data flow */
        for (int j = 0; j < 3; j++) {
            /* Use key_index inside inner loop */
            int inner_val = key_index + j;
            accumulator ^= inner_val;
            
            /* Use some of the high-pressure variables */
            v1 += j;
            d1 += j * 0.1;
        }
        
        /* Use computed values in opaque functions */
        use_int(v1);
        use_double(d1);
        use_int(val1 + val2);
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
