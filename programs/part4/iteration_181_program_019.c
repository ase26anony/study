/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * to execute the virtual register creation code in early-remat.cc
 */

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

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Create arrays to work with */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double farray[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
        farray[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Create many live variables to increase register pressure */
        int v1 = i * 2;
        int v2 = i * 3;
        int v3 = i * 5;
        int v4 = i * 7;
        int v5 = i * 11;
        int v6 = i * 13;
        int v7 = i * 17;
        int v8 = i * 19;
        
        double d1 = i * 1.1;
        double d2 = i * 1.3;
        double d3 = i * 1.7;
        double d4 = i * 1.9;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This should be a candidate for rematerialization */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        
        /* Second use: different array indexing */
        int val2 = array2[key_index];
        
        /* Third use: in conditional */
        if (key_index % 3 == 0) {
            /* Create inner control flow complexity */
            int inner_var = key_index * 2;
            use_int(inner_var);
            
            /* Fourth use: in floating point computation */
            double fval = farray[key_index] * 2.0;
            use_double(fval);
            
            /* More register pressure inside conditional */
            int v9 = inner_var * 2;
            int v10 = inner_var * 3;
            double d5 = fval * 1.5;
            
            /* Use all variables to prevent optimization */
            accumulator ^= v1 ^ v2 ^ v3 ^ v9 ^ v10;
            use_double(d5);
        } else if (key_index % 5 == 0) {
            /* Alternative path with different mode usage */
            double scaled = (double)key_index * 3.14159;
            use_double(scaled);
            
            /* Fifth use: pointer arithmetic */
            void* ptr = &array1[key_index];
            use_ptr(ptr);
        } else {
            /* Default path */
            int scaled_int = key_index * 11;
            use_int(scaled_int);
        }
        
        /* Sixth use: after conditional block */
        int final_use = array1[key_index] + array2[key_index];
        
        /* Complex inner loop to create cyclic data flow */
        for (int j = 0; j < 3; j++) {
            /* Seventh use: inside inner loop with different mode */
            double loop_val = farray[key_index] * j;
            accumulator += (int)loop_val;
            
            /* Mix in other variables */
            accumulator ^= v4 ^ v5 ^ v6;
            use_double(d1 + d2 + d3);
        }
        
        /* Use all remaining variables */
        accumulator += v7 + v8 + val1 + val2 + final_use;
        use_double(d4);
        
        /* Additional control flow with goto to create complexity */
        if (key_index % 7 == 0) {
            goto special_case;
        }
        
        continue;
        
    special_case:
        /* Eighth use: in goto target block */
        int special_val = array2[key_index] * 3;
        accumulator += special_val;
        use_int(key_index * 2);
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
