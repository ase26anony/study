/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
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

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = i * 7;
    }
    
    volatile int global_sink = 0;
    volatile double dbl_sink = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer / 2;
        int v5 = outer % 13;
        int v6 = v1 + v2;
        int v7 = v3 - v4;
        int v8 = v5 * 2;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = outer * 3.3;
        double d4 = d1 + d2;
        double d5 = d2 - d3;
        
        /* Key recomputable expression - used multiple times */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* First use of key_index - array access */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        (void)temp;
        
        /* Complex control flow splitting uses of key_index */
        if (outer % 3 == 0) {
            /* Second use in a different basic block */
            double val2 = array2[key_index];
            dbl_sink += val2;
            
            /* More register pressure variables */
            int v9 = v6 + v7;
            int v10 = v8 * 2;
            double d6 = d4 * d5;
            
            /* Use key_index again in conditional block */
            if (key_index % 2 == 0) {
                array3[key_index] = val1 + v9;
            } else {
                array3[key_index] = val1 - v10;
            }
            
            /* Call opaque function with key_index */
            use_int(key_index);
        } else if (outer % 3 == 1) {
            /* Alternative path with different mode usage */
            double scaled_index = (double)key_index;
            dbl_sink += scaled_index;
            
            /* Mixed mode computations */
            int v11 = (int)(scaled_index * 2.0);
            double d7 = scaled_index / 3.0;
            
            /* Third use of key_index */
            int *ptr = &array3[key_index];
            use_ptr(ptr);
            
            /* More computations to maintain pressure */
            v1 += v11;
            d1 += d7;
        } else {
            /* Third path with inner loop creating cyclic flow */
            int inner_sum = 0;
            for (int inner = 0; inner < 3; inner++) {
                /* Fourth use of key_index inside inner loop */
                inner_sum += array1[(key_index + inner) % ARRAY_SIZE];
                
                /* More variables to increase pressure */
                int v12 = v1 + inner;
                double d8 = d1 + inner;
                (void)v12;
                (void)d8;
            }
            
            /* Use key_index after inner loop */
            array2[key_index] = d1 + inner_sum;
        }
        
        /* Final use of key_index - ensures it's live across branches */
        global_sink ^= array1[key_index];
        
        /* Consume all variables to prevent optimization */
        global_sink += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        dbl_sink += d1 + d2 + d3 + d4 + d5;
        
        /* Additional recomputable expression with different mode */
        double recomputed_dbl = (double)outer * 3.14159;
        
        /* Multiple uses of double expression */
        dbl_sink += recomputed_dbl;
        if (recomputed_dbl > 1000.0) {
            dbl_sink -= recomputed_dbl * 0.1;
        }
        use_double(recomputed_dbl);
    }
    
    printf("Result: %d (sink: %f)\n", global_sink, dbl_sink);
    return global_sink != 0 ? 0 : 1;
}
