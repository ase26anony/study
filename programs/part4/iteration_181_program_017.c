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

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory references */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 1.5;
        array3[i] = ARRAY_SIZE - i;
    }
    
    volatile int accumulator = 0;
    
    /* High register pressure loop */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Create many live scalar variables to pressure registers */
        int v1 = i * 2;
        int v2 = i + 1;
        int v3 = i * 3;
        int v4 = i / 2;
        int v5 = i % 17;
        int v6 = i ^ 0x55;
        int v7 = i << 2;
        int v8 = i >> 1;
        
        double d1 = i * 1.1;
        double d2 = i * 2.2;
        double d3 = i * 3.3;
        double d4 = i * 4.4;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - candidate for rematerialization */
        /* This will be used multiple times in different contexts */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Second use: in arithmetic expression */
        int val2 = array3[key_index] * 2 + key_index;
        
        /* Third use: conditional check */
        if (key_index % 3 == 0) {
            /* Create inner control flow complexity */
            volatile int inner_sink = key_index;
            val1 += inner_sink;
            
            /* Use key_index again in this branch */
            double temp = array2[key_index] * d1;
            use_double(temp);
        } else if (key_index % 5 == 0) {
            /* Alternative branch with different use */
            int temp = key_index * key_index;
            use_int(temp);
        }
        
        /* Fourth use: function call argument */
        use_int(key_index);
        
        /* Fifth use: pointer arithmetic (different mode) */
        void *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Mix in the other variables to keep them live */
        v1 += val1;
        v2 += val2;
        d1 += array2[key_index % 256];
        
        /* Complex control flow with goto to create cycles */
        if (key_index % 7 == 0) {
            /* Small inner loop to challenge liveness analysis */
            for (int j = 0; j < 3; j++) {
                /* Use key_index inside inner loop */
                int inner_val = key_index + j;
                v3 += inner_val;
                
                /* Use double variables too */
                d2 += j * 0.5;
            }
        }
        
        /* Use all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        accumulator ^= (int)d1 ^ (int)d2 ^ (int)d3 ^ (int)d4;
        
        /* Force materialization with volatile */
        volatile int force_materialize = key_index;
        (void)force_materialize;
        
        /* Use key_index one more time at end of iteration */
        if (i % 1000 == 0) {
            /* This creates another use point */
            array1[key_index] = i;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
