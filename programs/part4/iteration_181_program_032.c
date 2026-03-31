/* early-remat-trigger.c */
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

int main(void) {
    const int ARRAY_SIZE = 1000;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray1[ARRAY_SIZE];
    double darray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        darray1[i] = i * 0.5;
        darray2[i] = i * 1.5;
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
        
        double d1 = i * 0.1;
        double d2 = i * 0.2;
        double d3 = i * 0.3;
        double d4 = i * 0.4;
        double d5 = i * 0.5;
        double d6 = i * 0.6;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array with offset */
        int val2 = array2[(key_index + 5) % ARRAY_SIZE];
        
        /* Third use: in conditional */
        if (key_index % 3 == 0) {
            /* Use key_index again inside branch */
            double dval = darray1[key_index];
            use_double(dval);
            
            /* More computations to increase pressure */
            int v9 = v1 + v2;
            int v10 = v3 + v4;
            volatile int sink2 = v9 + v10;
        } else if (key_index % 3 == 1) {
            /* Alternative path using key_index */
            double dval = darray2[key_index];
            use_double(dval);
            
            /* Different computations */
            int v11 = v5 * v6;
            volatile int sink3 = v11;
        } else {
            /* Third path - create more register pressure */
            int v12 = v7 ^ v8;
            volatile int sink4 = v12;
            
            /* Use key_index in pointer arithmetic */
            void *ptr = &array1[key_index];
            use_ptr(ptr);
        }
        
        /* Fourth use: after conditional, in another computation */
        int val3 = val1 + val2 + key_index;
        
        /* Fifth use: pass to dummy function */
        use_int(key_index);
        
        /* Sixth use: in another array access with different mode */
        double dval2 = darray1[key_index] + darray2[key_index];
        use_double(dval2);
        
        /* Mix integer and double computations to create different modes */
        double mixed = d1 + d2 + key_index;
        volatile double sink5 = mixed;
        
        /* Complex control flow with goto to create cycles */
        if (key_index % 7 == 0) {
            /* Small inner "loop" using goto */
            int counter = 3;
        inner_loop:
            if (counter > 0) {
                /* Use key_index inside goto loop */
                int temp = array1[(key_index + counter) % ARRAY_SIZE];
                volatile int sink6 = temp;
                counter--;
                goto inner_loop;
            }
        }
        
        /* More register pressure variables */
        int v13 = v1 ^ v2 ^ v3;
        int v14 = v4 + v5 + v6;
        int v15 = v7 * v8;
        double d7 = d1 * d2;
        double d8 = d3 / d4;
        
        /* Use all variables to prevent dead code elimination */
        volatile int sink7 = v13 + v14 + v15;
        volatile double sink8 = d5 + d6 + d7 + d8;
        
        /* Update accumulator to prevent optimization */
        accumulator ^= val3 + key_index;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
