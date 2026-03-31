/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int val) {
    static volatile int sink;
    sink = val;
    return sink & 1;
}

double __attribute__((noinline)) use_double(double val) {
    static volatile double sink;
    sink = val;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(void *ptr) {
    static volatile void *sink;
    sink = ptr;
}

/* Main function creating high register pressure */
int main(void) {
    /* Large arrays to create memory pressure */
    int array1[1024];
    double array2[1024];
    int array3[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = i * 7;
    }
    
    volatile int accumulator = 0;
    const int iter_count = 100000;
    
    /* High register pressure loop */
    for (int outer = 0; outer < iter_count; outer++) {
        /* Create many live variables to pressure registers */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer / 2;
        int v5 = outer % 17;
        int v6 = v1 + v2;
        int v7 = v3 - v4;
        int v8 = v5 * 2;
        int v9 = v6 + v7;
        int v10 = v8 - v9;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = outer * 3.3;
        double d4 = d1 + d2;
        double d5 = d3 - d1;
        double d6 = d4 * d5;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % 1024;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: in conditional with complex control flow */
        if (key_index % 3 == 0) {
            /* Inner block that uses key_index again */
            double temp = array2[key_index];
            d6 += temp;
            
            /* More variables to increase pressure */
            int v11 = key_index * 2;
            int v12 = v11 + v10;
            accumulator ^= v12;
            
            /* Nested conditional */
            if (key_index % 5 == 0) {
                int v13 = key_index + v9;
                use_int(v13);
            }
        } else if (key_index % 7 == 0) {
            /* Alternative path using key_index */
            int v14 = key_index * 3;
            use_int(v14);
            
            /* Create a small inner loop for cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                int v15 = key_index + inner;
                accumulator += v15;
            }
        } else {
            /* Default path */
            int v16 = key_index % 11;
            accumulator -= v16;
        }
        
        /* Third use: function call argument */
        use_int(key_index);
        
        /* Fourth use: array indexing with different array */
        int val3 = array3[key_index];
        
        /* Fifth use: in arithmetic expression */
        int v17 = (key_index * 13) / 7;
        
        /* Use all variables to prevent optimization */
        int sum_int = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 + 
                     val1 + val3 + v17;
        accumulator += sum_int;
        
        double sum_double = d1 + d2 + d3 + d4 + d5 + d6;
        accumulator += (int)sum_double;
        
        /* Use double version of key computation */
        double key_double = key_index * 1.234;
        use_double(key_double);
        
        /* Switch statement to create more control flow */
        switch (key_index % 4) {
            case 0:
                accumulator ^= (key_index * 2);
                break;
            case 1:
                accumulator ^= (key_index * 3);
                /* Fall through */
            case 2:
                accumulator ^= (key_index * 5);
                break;
            default:
                accumulator ^= (key_index * 7);
                break;
        }
        
        /* Pointer computation using key_index */
        void *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Final use of key_index in this iteration */
        if (key_index > 512) {
            accumulator += 1;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0;
}
