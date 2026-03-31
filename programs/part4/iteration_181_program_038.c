/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 1000000

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
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to pressure registers */
        int v1 = outer * 2;
        int v2 = outer * 3;
        int v3 = outer * 5;
        int v4 = outer * 7;
        int v5 = outer * 11;
        int v6 = outer * 13;
        int v7 = outer * 17;
        int v8 = outer * 19;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        double d6 = outer * 0.6;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index] + v1;
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array indexing */
        double val2 = darray1[key_index] + d1;
        
        /* Third use: in conditional */
        if (key_index % 3 == 0) {
            val1 += array2[key_index];
            /* Use more variables to maintain pressure */
            v2 += v3;
            d2 += d3;
        } else if (key_index % 5 == 0) {
            /* Alternative path that also uses key_index */
            val2 += darray2[key_index];
            v4 += v5;
            d4 += d5;
        } else {
            /* Default path with goto to create complex control flow */
            v6 += v7;
            d6 += d1;
        }
        
        /* Fourth use: passed to function */
        use_int(key_index);
        
        /* Inner conditional block splitting uses across blocks */
        if (outer % 2 == 0) {
            /* Fifth use: recompute-like usage */
            int temp_key = (outer * 7 + 123) % ARRAY_SIZE; /* Same computation! */
            if (temp_key != key_index) {
                /* Should never happen, but prevents CSE */
                use_int(temp_key);
            }
            
            /* Use in pointer arithmetic */
            int* ptr = &array1[key_index];
            use_ptr(ptr);
            
            /* More register pressure */
            int v9 = v1 + v2;
            int v10 = v3 + v4;
            double d7 = d1 + d2;
            double d8 = d3 + d4;
            
            /* Consume them */
            sink1 = v9 + v10;
            volatile double sink2 = d7 + d8;
        } else {
            /* Different path using key_index again */
            double val3 = darray2[key_index] * 2.0;
            use_double(val3);
            
            /* Create cyclic data flow with goto */
            int counter = 0;
        loop_label:
            if (counter++ < 2) {
                /* Use key_index inside mini-loop */
                int offset = key_index + counter;
                if (offset >= ARRAY_SIZE) offset = 0;
                val1 += array1[offset];
                goto loop_label;
            }
        }
        
        /* Mix data types and modes */
        if (key_index % 7 == 0) {
            /* Use double computation */
            double dkey = (double)key_index * 1.234;
            use_double(dkey);
            
            /* And int computation */
            int ikey = key_index * 11;
            use_int(ikey);
        }
        
        /* Final accumulation to prevent elimination */
        accumulator ^= val1;
        accumulator ^= (int)val2;
        accumulator ^= v8;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
