/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
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
    return sink * 1.01;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1000;
    const int ITERATIONS = 100000;
    
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
    
    volatile int global_sink = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 3;
        int v2 = outer + 17;
        int v3 = outer / 2;
        int v4 = outer - 42;
        int v5 = outer * 7;
        int v6 = outer + 99;
        int v7 = outer ^ 0x55;
        int v8 = outer | 0xAA;
        
        double d1 = outer * 0.3;
        double d2 = outer * 0.7;
        double d3 = outer * 1.3;
        double d4 = outer * 2.7;
        double d5 = outer * 3.14;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - candidate for rematerialization */
        /* This will be used multiple times in different contexts */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        (void)temp;
        
        /* Second use: in conditional with complex control flow */
        if (key_index % 3 == 0) {
            /* Create inner control flow with goto to challenge analysis */
            if (key_index % 5 == 0) {
                goto inner_block;
            }
            
            /* Use key_index in array access */
            double val2 = array2[key_index];
            d1 += val2;
            
            inner_block:
            /* Third use: in arithmetic */
            v1 += key_index * 2;
            
            /* Fourth use: as function argument */
            use_int(key_index);
        } else if (key_index % 7 == 0) {
            /* Alternative path using key_index */
            v2 -= key_index;
            
            /* Switch statement to create more control flow */
            switch (key_index % 4) {
                case 0:
                    v3 = key_index + array3[key_index];
                    break;
                case 1:
                    v4 = key_index - array1[key_index];
                    break;
                default:
                    v5 = key_index * 3;
            }
        } else {
            /* Another path using key_index differently */
            v6 = key_index | 0xFF;
        }
        
        /* Fifth use: after conditional blocks */
        int val3 = array3[key_index];
        
        /* Mix with double computations to create different modes */
        double recomputed_double = key_index * 2.5;
        d2 += recomputed_double;
        
        /* Use in pointer arithmetic (different mode) */
        void *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* More register pressure variables */
        int v9 = v1 + v2;
        int v10 = v3 * v4;
        int v11 = v5 ^ v6;
        int v12 = v7 & v8;
        
        double d6 = d1 + d2;
        double d7 = d3 * d4;
        double d8 = d5 / 2.0;
        
        /* Consume all variables to prevent optimization */
        global_sink ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10 ^ v11 ^ v12;
        global_sink ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4 ^ (int)d5;
        global_sink ^= (int)d6 ^ (int)d7 ^ (int)d8;
        global_sink ^= val1 ^ val3 ^ key_index;
        
        /* Inner loop to create cyclic data flow */
        for (int inner = 0; inner < 3; inner++) {
            /* Recompute key_index again (another use) */
            int inner_key = key_index + inner;
            v1 += array1[inner_key % ARRAY_SIZE];
            
            /* More double computations */
            d1 += array2[inner_key % ARRAY_SIZE];
        }
        
        /* Final use of key_index before loop ends */
        if (key_index > ARRAY_SIZE / 2) {
            v2 += key_index;
        }
    }
    
    printf("Result: %d\n", global_sink);
    return global_sink != 0 ? 0 : 1;
}
