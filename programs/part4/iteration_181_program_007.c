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

__attribute__((noinline)) void use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = i * 7;
    }
    
    volatile int accumulator = 0;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Create many live scalar variables to increase register pressure */
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
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index multiple times in different contexts */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Complex control flow that splits uses */
        if (i % 3 == 0) {
            /* Second use: different array with same index */
            double val2 = array2[key_index];
            
            /* Third use: arithmetic operation */
            int val3 = array3[key_index] + key_index;
            
            /* Use volatile to prevent optimization */
            volatile double sink2 = val2;
            volatile int sink3 = val3;
            
            /* Call dummy function with the value */
            use_int(key_index);
            
            /* Inner conditional creating more control flow */
            if (val3 % 2 == 0) {
                /* Fourth use: pointer arithmetic */
                void *ptr = &array1[key_index];
                use_ptr(ptr);
                
                /* Mix with double computation */
                double temp = d1 + d2 + key_index;
                use_double(temp);
            } else {
                /* Alternative path still using key_index */
                int temp2 = v1 + v2 + key_index;
                use_int(temp2);
            }
            
            /* Small inner loop creating cyclic data flow */
            for (int j = 0; j < 3; j++) {
                /* Fifth use: recomputed in inner loop */
                int inner_val = array1[key_index] + j;
                accumulator ^= inner_val;
            }
        } else if (i % 3 == 1) {
            /* Different branch, still using key_index */
            int val4 = array3[key_index] - key_index;
            use_int(val4);
            
            /* Use with double mode */
            double mixed = key_index * 1.5 + d3;
            use_double(mixed);
        } else {
            /* Third branch with switch statement */
            switch (key_index % 4) {
                case 0:
                    use_int(key_index + v3);
                    break;
                case 1:
                    use_double(key_index * 0.5);
                    break;
                case 2:
                    use_int(key_index * 2);
                    break;
                default:
                    /* Use in goto label creating more complex CFG */
                    alternate_use:
                    use_int(key_index + v4);
                    break;
            }
            
            /* Jump to label to create non-trivial control flow */
            if (key_index % 8 == 0) {
                goto alternate_use;
            }
        }
        
        /* Sixth use: after control flow rejoins */
        int final_use = array1[key_index] * 2;
        accumulator += final_use;
        
        /* Use all the pressure variables to keep them live */
        int sum_ints = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        double sum_doubles = d1 + d2 + d3 + d4;
        
        /* Volatile operations to prevent dead code elimination */
        volatile int int_sink = sum_ints;
        volatile double double_sink = sum_doubles;
        
        /* Mix with key_index one more time */
        accumulator ^= key_index;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
