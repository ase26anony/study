/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
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
    return sink * 1.1;
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
    int *array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = &array1[i];
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
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
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Second use: in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            /* Third use: different array indexing */
            double val2 = array2[key_index];
            d_accumulator += val2;
            
            /* Fourth use: pointer arithmetic */
            int *ptr = array3[key_index];
            use_ptr(ptr);
            
            /* Fifth use: in arithmetic */
            int val3 = key_index * 2 + 1;
            accumulator ^= val3;
        } else {
            /* Sixth use: in else branch */
            int val4 = key_index / 2;
            accumulator += val4;
            
            /* Seventh use: function argument */
            use_int(key_index);
        }
        
        /* Eighth use: after conditional, with volatile to force materialization */
        volatile int sink_key = key_index;
        accumulator += sink_key;
        
        /* Create complex control flow with goto to challenge liveness analysis */
        if (v10 % 13 == 0) {
            /* Inner loop-like structure */
            int inner_temp = 0;
            for (int j = 0; j < 3; j++) {
                /* Ninth use: recompute key_index inside inner loop */
                int recomputed_key = (outer * 7 + 123) % ARRAY_SIZE;
                inner_temp += recomputed_key;
                
                /* Use double variables to create mixed mode pressure */
                d_accumulator += d6 * j;
            }
            accumulator += inner_temp;
            
            /* Tenth use: after inner loop */
            use_int(key_index + 1);
        }
        
        /* Use all the variables to keep them live */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        d_accumulator += d1 + d2 + d3 + d4 + d5 + d6;
        
        /* Force another recomputation with different mode (double) */
        double key_double = (double)((outer * 11 + 456) % ARRAY_SIZE);
        d_accumulator += key_double;
        
        /* Use in switch statement for additional control flow complexity */
        switch (key_index % 5) {
            case 0:
                accumulator += array1[key_index % 100];
                break;
            case 1:
                accumulator -= key_index;
                break;
            case 2:
                /* Eleventh use: recompute again */
                use_int((outer * 7 + 123) % ARRAY_SIZE);
                break;
            case 3:
                d_accumulator += key_double * 2.0;
                break;
            case 4:
                accumulator ^= key_index;
                break;
        }
    }
    
    printf("Result: %d (int), %.2f (double)\n", accumulator, d_accumulator);
    return accumulator > 0 ? 0 : 1;
}
