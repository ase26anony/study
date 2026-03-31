/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink __attribute__((unused)) = x;
}

__attribute__((noinline)) void use_double(double x) {
    volatile double sink __attribute__((unused)) = x;
}

__attribute__((noinline)) void use_ptr(void *p) {
    volatile void *sink __attribute__((unused)) = p;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1000;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 7;
        darray[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
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
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array with offset */
        int val2 = array2[(key_index + 5) % ARRAY_SIZE];
        
        /* Third use: conditional check */
        if (key_index % 3 == 0) {
            /* Create inner control flow complexity */
            int temp = v1 + v2 + v3;
            if (temp % 2 == 0) {
                val1 += array1[(key_index + 1) % ARRAY_SIZE];
            } else {
                /* Use key_index again in else branch */
                val2 += array2[(key_index + 2) % ARRAY_SIZE];
            }
            
            /* More computations to increase pressure */
            double dtemp = d1 + d2 + d3;
            use_double(dtemp);
        }
        
        /* Fourth use: passed to dummy function */
        use_int(key_index);
        
        /* Fifth use: in pointer arithmetic */
        int *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Sixth use: in double array with type mixing */
        double dval = darray[key_index];
        use_double(dval);
        
        /* Create cyclic data flow with goto to challenge liveness analysis */
        if (key_index % 5 == 0) {
            /* Small inner loop to create more complex CFG */
            int inner_sum = 0;
            for (int inner = 0; inner < 3; inner++) {
                /* Use key_index inside inner loop */
                inner_sum += array1[(key_index + inner) % ARRAY_SIZE];
                
                /* Mix with other live variables */
                v4 += inner;
                d1 += 0.1;
            }
            accumulator ^= inner_sum;
        }
        
        /* Seventh use: after inner control flow */
        int val3 = array1[(key_index * 2) % ARRAY_SIZE];
        
        /* Consume all variables to prevent optimization */
        accumulator += val1 + val2 + val3 + v4 + v5 + v6 + v7 + v8;
        accumulator ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4;
        
        /* More register pressure with mixed modes */
        if (outer % 100 == 0) {
            /* Force different machine modes (SI for int, DF for double) */
            long long big_val = (long long)key_index * key_index;
            volatile long long sink2 = big_val;
            
            float fval = key_index * 0.5f;
            volatile float sink3 = fval;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
