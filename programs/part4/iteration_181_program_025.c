/* Program to trigger early rematerialization virtual register creation */
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

__attribute__((noinline)) void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Create arrays to create memory pressure */
    int* int_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* double_array = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        double_array[i] = i * 1.5;
    }
    
    volatile int global_sink = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer * 3;
        int v3 = outer * 5;
        int v4 = outer * 7;
        int v5 = outer * 11;
        int v6 = outer * 13;
        int v7 = outer * 17;
        int v8 = outer * 19;
        
        double d1 = outer * 1.1;
        double d2 = outer * 1.3;
        double d3 = outer * 1.7;
        double d4 = outer * 1.9;
        double d5 = outer * 2.1;
        double d6 = outer * 2.3;
        double d7 = outer * 2.7;
        double d8 = outer * 2.9;
        
        /* This is the recomputable expression - cheap but used multiple times */
        /* The expression uses different operations to create different RTL patterns */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use the recomputable value in multiple places */
        /* First use: array indexing */
        int val1 = int_array[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        (void)temp;
        
        /* Second use: in a conditional with complex control flow */
        if (key_index % 3 == 0) {
            /* Create inner control flow with goto to make analysis non-trivial */
            if (key_index % 5 == 0) {
                goto inner_block;
            }
            
            double val2 = double_array[key_index];
            use_double(val2);
            
            /* Third use: in arithmetic */
            int offset = key_index * 2;
            v1 += offset;
            
            inner_block:
            /* Fourth use: function call */
            use_int(key_index);
            
            /* Mix in the double variables to create different modes */
            d1 += key_index;
            use_double(d1);
        } else if (key_index % 7 == 0) {
            /* Alternative path that also uses key_index */
            int* ptr = &int_array[key_index];
            use_ptr(ptr);
            
            /* Fifth use: in another array access */
            double val3 = double_array[key_index / 2];
            use_double(val3);
        } else {
            /* Default path */
            v2 += key_index;
        }
        
        /* Create a small inner loop to increase complexity */
        int inner_sum = 0;
        for (int inner = 0; inner < 3; inner++) {
            /* Sixth use: inside inner loop */
            inner_sum += key_index + inner;
            
            /* Use many variables to maintain pressure */
            v3 += v4 + v5;
            d2 += d3 + d4;
        }
        
        /* Seventh use: after inner loop */
        v6 += inner_sum * key_index;
        
        /* Use all variables to prevent dead code elimination */
        v7 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        d5 = d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
        
        /* Force materialization */
        volatile int sink1 = v7;
        volatile double sink2 = d5;
        
        /* Accumulate to global sink */
        global_sink ^= v7 + (int)d5;
        
        /* Eighth use: final array access with key_index */
        int_array[key_index % 256] = outer;
    }
    
    printf("Result: %d\n", global_sink);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    
    return 0;
}
