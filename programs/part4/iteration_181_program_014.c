/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>

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

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Create arrays to work with */
    int *int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        double_array[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer + 4;
        int v5 = outer * 5;
        int v6 = outer + 6;
        int v7 = outer * 7;
        int v8 = outer + 8;
        int v9 = outer * 9;
        int v10 = outer + 10;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = outer * 3.3;
        double d4 = outer * 4.4;
        double d5 = outer * 5.5;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = int_array[key_index];
        
        /* Second use: arithmetic operation */
        int val2 = key_index * 2 + 1;
        
        /* Third use: conditional check */
        if (key_index % 3 == 0) {
            /* Use key_index again inside branch */
            double val3 = double_array[key_index];
            use_double(val3);
            
            /* More register pressure inside branch */
            int v11 = v1 + key_index;
            int v12 = v2 * key_index;
            use_int(v11);
            use_int(v12);
        }
        
        /* Fourth use: passed to function */
        use_int(key_index);
        
        /* Fifth use: in another array access */
        if (key_index > ARRAY_SIZE / 2) {
            int_array[key_index] = val1 + val2;
        }
        
        /* Complex control flow with goto to create cycles */
        if (outer % 7 == 0) {
            /* Small inner loop to create data flow complexity */
            int inner = 0;
            while (inner < 3) {
                /* Use key_index again inside inner loop */
                int temp = key_index + inner;
                use_int(temp);
                inner++;
            }
            
            /* Use computed values to prevent elimination */
            accumulator ^= v1 ^ v3 ^ v5 ^ v7 ^ v9;
            accumulator += val1 + val2;
        } else if (outer % 5 == 0) {
            /* Another branch using the values */
            goto compute_label;
        } else {
            /* Default path */
            use_double(d1 + d2 + d3);
        }
        
        /* Label for goto to create control flow complexity */
        compute_label:
        {
            /* Use key_index one more time after label */
            volatile int sink_key = key_index;
            (void)sink_key;
            
            /* Mix in double computations for different modes */
            double mixed = d4 * key_index + d5;
            use_double(mixed);
        }
        
        /* Prevent optimization of all variables */
        use_int(v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10);
        use_double(d1 + d2 + d3 + d4 + d5);
        
        /* Use pointer to force address mode */
        if (outer % 11 == 0) {
            use_ptr(&int_array[key_index]);
        }
    }
    
    printf("Result: %d\n", accumulator);
    
    free(int_array);
    free(double_array);
    
    return 0;
}
