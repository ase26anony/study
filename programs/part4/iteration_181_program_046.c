/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    static volatile int sink;
    sink = x;
    return sink;
}

double __attribute__((noinline)) use_double(double x) {
    static volatile double sink;
    sink = x;
    return sink;
}

void __attribute__((noinline)) use_ptr(void *p) {
    static volatile void *sink;
    sink = p;
}

/* Main function creating high register pressure with recomputable values */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = ARRAY_SIZE - i;
    }
    
    volatile int accumulator = 0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to pressure registers */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer - 5;
        int v5 = outer / 2;
        int v6 = outer % 7;
        int v7 = outer << 1;
        int v8 = outer >> 1;
        int v9 = outer ^ 0x55;
        int v10 = outer | 0xAA;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - key candidate for rematerialization */
        /* This should create a REG in SI mode */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Another recomputable expression in DF mode */
        double key_scale = (outer * 0.07 + 1.23);
        
        /* Use key_index in multiple places - creates multiple DF_REFs */
        int val1 = array1[key_index];          /* First use */
        double val2 = array2[key_index];       /* Second use */
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Complex control flow splitting uses of recomputable value */
        if (outer % 3 == 0) {
            /* Use key_index again in different basic block */
            int val3 = array3[key_index];      /* Third use */
            accumulator ^= val3;
            
            /* Inner conditional creating more data flow */
            if (key_index % 2 == 0) {          /* Fourth use */
                accumulator += val1;
            } else {
                accumulator -= val2;
            }
            
            /* Small inner loop creating cyclic data flow */
            int inner_sum = 0;
            for (int j = 0; j < 3; j++) {
                inner_sum += key_index + j;    /* Fifth use */
            }
            accumulator += inner_sum;
            
        } else if (outer % 3 == 1) {
            /* Alternative path using the recomputable value */
            use_int(key_index);                /* Sixth use */
            
            /* Use key_scale (DF mode) in this path */
            double scaled = key_scale * d1;
            use_double(scaled);
            
            /* More register pressure variables */
            int t1 = v1 + v2;
            int t2 = v3 * v4;
            int t3 = v5 ^ v6;
            int t4 = v7 | v8;
            int t5 = v9 & v10;
            
            accumulator += t1 + t2 + t3 + t4 + t5;
            
        } else {
            /* Third path with goto creating non-trivial control flow */
            if (key_index < ARRAY_SIZE / 2) {  /* Seventh use */
                goto process_small;
            }
            
            /* Use in array computation */
            array1[key_index] = val1 + 1;      /* Eighth use */
            
            process_small:
            /* Use key_index after label */
            use_ptr(&array1[key_index]);       /* Ninth use */
        }
        
        /* Use recomputable expressions in mixed operations */
        int combined = key_index + (int)key_scale;  /* Mixed mode use */
        accumulator ^= combined;
        
        /* More volatile operations to prevent optimization */
        volatile double sink2 = key_scale;
        volatile int sink3 = val1;
        
        /* Use all pressure variables to keep them live */
        int sum_vars = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        double sum_doubles = d1 + d2 + d3 + d4 + d5;
        
        accumulator += sum_vars;
        accumulator += (int)sum_doubles;
        
        /* Prevent loop unrolling */
        if (outer % 100 == 0) {
            use_int(accumulator);
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
