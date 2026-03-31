/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy noinline functions to prevent optimization */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray1[ARRAY_SIZE];
    double darray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        darray1[i] = i * 0.5;
        darray2[i] = (ARRAY_SIZE - i) * 0.25;
    }
    
    volatile int accumulator = 0;
    volatile double daccumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 3;
        int v2 = outer + 17;
        int v3 = outer / 2;
        int v4 = outer % 13;
        int v5 = v1 + v2;
        int v6 = v3 - v4;
        int v7 = v5 * v6;
        int v8 = v7 & 0xFF;
        
        double d1 = outer * 0.3;
        double d2 = outer + 2.718;
        double d3 = d1 * d2;
        double d4 = d3 / 1.414;
        double d5 = d4 - d1;
        double d6 = d5 + d2;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        int val2 = array2[key_index];
        
        /* Second use: in conditional expression */
        if (key_index % 3 == 0) {
            v1 += val1;
            d1 += darray1[key_index];
        } else if (key_index % 3 == 1) {
            v2 += val2;
            d2 += darray2[key_index];
        } else {
            /* Third use: in arithmetic */
            int offset = key_index * 2;
            v3 += offset;
            d3 += offset * 0.1;
        }
        
        /* Fourth use: passed to dummy function */
        use_int(key_index);
        
        /* Complex control flow with inner conditional */
        switch (key_index % 5) {
            case 0:
                /* Fifth use: different mode (pointer arithmetic) */
                use_ptr(&array1[key_index]);
                v4 += key_index;
                break;
            case 1:
                /* Sixth use: in double computation */
                d4 += key_index * 0.01;
                v5 += key_index / 2;
                break;
            case 2:
                /* Create inner loop-like structure with goto */
                {
                    int temp = key_index;
                inner_label:
                    if (temp > 0) {
                        /* Seventh use: in loop-like computation */
                        v6 += temp;
                        temp--;
                        goto inner_label;
                    }
                }
                break;
            case 3:
                /* Eighth use: in array access on both sides */
                array1[key_index] = array2[key_index] + key_index;
                break;
            case 4:
                /* Ninth use: multiple times in expression */
                v7 = (key_index * key_index) / (key_index + 1);
                break;
        }
        
        /* More register pressure variables */
        int v9 = v8 + key_index;  /* Tenth use */
        int v10 = v9 * 2;
        double d7 = d6 + key_index * 0.001;  /* Different mode use */
        
        /* Use all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        daccumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7;
        
        /* Force materialization with volatile */
        volatile int sink_int = key_index;  /* Eleventh use */
        volatile double sink_double = key_index * 0.5;  /* Different mode */
        
        /* Use dummy functions */
        use_int(v10);
        use_double(d7);
        
        /* Additional branching to split live ranges */
        if (outer % 100 == 0) {
            /* Twelfth use: in rare path */
            int rare = key_index * 3;
            use_int(rare);
        }
    }
    
    printf("Result: %d (int), %f (double)\n", accumulator, daccumulator);
    return 0;
}
