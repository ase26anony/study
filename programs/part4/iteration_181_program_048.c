/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink & 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Dummy computation to create recomputable expressions */
int __attribute__((noinline)) compute_key(int i, int base) {
    /* Cheap, pure computation - candidate for rematerialization */
    return (i * 7 + base) & 0xFFF;
}

double __attribute__((noinline)) compute_scale(int i, double factor) {
    /* Another cheap computation in different mode (DF) */
    return (i & 0xF) * factor;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 100000;
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* Arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int *ptr_array[ARRAY_SIZE/4];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        if (i % 4 == 0) ptr_array[i/4] = &array1[i];
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer;
        int v2 = outer + 1;
        int v3 = outer * 2;
        int v4 = outer / 3;
        int v5 = outer ^ 0x1234;
        int v6 = outer & 0xFF;
        int v7 = outer | 0xAA;
        int v8 = outer << 2;
        int v9 = outer >> 1;
        int v10 = -outer;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* KEY COMPUTATION: Cheap recomputable expression */
        /* This is the value we want to be rematerialized */
        int key_index = compute_key(outer, 123);
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: conditional check */
        if (key_index > ARRAY_SIZE / 2) {
            v1 += array1[key_index - ARRAY_SIZE/2];
        } else {
            v2 += array1[key_index + ARRAY_SIZE/2];
        }
        
        /* Third use: pointer arithmetic */
        if (key_index % 4 == 0) {
            use_ptr(ptr_array[key_index/4]);
        }
        
        /* Complex control flow with inner conditional */
        switch (outer % 5) {
            case 0:
                /* Use key_index again in different basic block */
                v3 += key_index * 2;
                break;
            case 1:
                v4 += key_index / 2;
                /* Create inner cyclic flow with goto */
                if (key_index % 3 == 0) {
                    v5 += key_index;
                }
                break;
            case 2:
                /* Another use of key_index */
                v6 ^= key_index;
                break;
            case 3:
                /* Use in floating point context */
                d1 += key_index * 0.25;
                break;
            default:
                /* Multiple uses in same statement */
                v7 = (key_index + v8) * key_index;
                break;
        }
        
        /* Fourth use: function argument */
        v9 += use_int(key_index);
        
        /* Create another recomputable value in different mode (DF) */
        double scale = compute_scale(outer, 3.14159);
        
        /* Use the double value multiple times */
        d2 += scale;
        array2[outer % ARRAY_SIZE] = scale * d3;
        
        if (outer % 7 == 0) {
            d4 = use_double(scale) * d5;
        }
        
        /* More register pressure variables */
        int v11 = v1 + v2;
        int v12 = v3 - v4;
        int v13 = v5 * v6;
        int v14 = v7 ^ v8;
        int v15 = v9 | v10;
        
        double d6 = d1 + d2;
        double d7 = d3 * d4;
        double d8 = d5 - d1;
        
        /* Use all variables to prevent optimization */
        accumulator ^= v11 + v12 + v13 + v14 + v15;
        fp_accumulator += d6 + d7 + d8;
        
        /* Inner loop to create more complex control flow */
        for (int inner = 0; inner < 3; inner++) {
            /* Use key_index again inside inner loop */
            int temp = key_index + inner;
            accumulator += temp;
            
            /* More computations to increase pressure */
            double d_temp = scale * inner;
            fp_accumulator += d_temp;
            
            /* Conditional that splits uses */
            if (inner % 2 == 0) {
                v1 += temp;
            } else {
                v2 += temp;
            }
        }
        
        /* Final use of key_index before loop ends */
        v10 += key_index % 17;
        
        /* Prevent everything from being optimized away */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        fp_accumulator += d1 + d2 + d3 + d4 + d5;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d (%.2f)\n", accumulator, fp_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
