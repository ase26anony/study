/* early-remat-trigger.c
 * Designed to trigger uncovered lines in early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

/* Dummy functions to prevent optimization */
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

/* Complex control flow helper */
__attribute__((noinline)) int conditional_transform(int x, int y) {
    if (x > y) {
        return x * 3 - y;
    } else {
        return y * 2 + x;
    }
}

int main(void) {
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
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Create many live variables to increase register pressure */
        int v1 = i * 2;
        int v2 = i + 123;
        int v3 = i % 17;
        int v4 = i / 3;
        int v5 = 255 - (i & 0xFF);
        int v6 = v1 + v2;
        int v7 = v3 * v4;
        int v8 = v5 ^ v6;
        
        double d1 = i * 0.1;
        double d2 = i * 0.2;
        double d3 = i * 0.3;
        double d4 = d1 + d2;
        double d5 = d2 * d3;
        double d6 = d4 - d5;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index] + v1;
        
        /* Second use: conditional check */
        if (key_index > ARRAY_SIZE / 2) {
            /* Third use: different array indexing */
            int val2 = array2[key_index] * 2;
            v8 += val2;
            
            /* Fourth use: in arithmetic */
            v7 = key_index / 4 + v7;
            
            /* Complex control flow with inner conditional */
            switch (key_index % 5) {
                case 0:
                    v6 = key_index + v6;  /* Fifth use */
                    break;
                case 1:
                    v5 = key_index - v5;  /* Sixth use */
                    break;
                case 2:
                    /* Seventh use: pointer arithmetic */
                    use_ptr(&array1[key_index]);
                    break;
                default:
                    v4 = key_index * 3;  /* Eighth use */
                    break;
            }
        } else {
            /* Alternative path that also uses key_index */
            /* Ninth use: double array indexing with different mode */
            double dval = darray1[key_index] + d1;
            d_accumulator += dval;
            
            /* Tenth use: in function call */
            use_int(key_index);
        }
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;  /* Eleventh use */
        (void)sink1;
        
        /* More computations to keep variables live */
        int v9 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        double d7 = d1 + d2 + d3 + d4 + d5 + d6;
        
        /* Use dummy functions to prevent optimization */
        use_int(v9);
        use_double(d7);
        
        /* Use key_index one more time before loop ends */
        /* Twelfth use: in conditional_transform */
        int transformed = conditional_transform(key_index, v9);
        
        /* Update accumulators to prevent dead code elimination */
        accumulator ^= transformed;
        d_accumulator += d7;
        
        /* Inner loop to create cyclic data flow */
        for (int j = 0; j < 3; j++) {
            /* Use key_index inside inner loop */
            /* Thirteenth use: creates complex liveness */
            int inner_val = key_index * j;
            accumulator += inner_val;
            
            /* More register pressure in inner loop */
            double d_inner = darray2[key_index] * j;
            d_accumulator -= d_inner;
        }
        
        /* Use goto to create additional control flow complexity */
        if (i % 100 == 0) {
            goto special_case;
        }
        continue;
        
    special_case:
        /* Fourteenth use: in special case block */
        array1[key_index] = accumulator % 100;
        /* Back to loop */
    }
    
    printf("Result: int=%d, double=%f\n", accumulator, d_accumulator);
    return 0;
}
