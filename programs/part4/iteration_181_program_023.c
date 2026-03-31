/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
#define NOINLINE __attribute__((noinline, noclone))

NOINLINE void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

NOINLINE void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

NOINLINE void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
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
        darray2[i] = (ARRAY_SIZE - i) * 0.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer + 4;
        int v5 = outer * 5;
        int v6 = outer + 6;
        int v7 = outer * 7;
        int v8 = outer + 8;
        int v9 = outer * 9;
        int v10 = outer + 10;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        use_int(val1);
        
        /* Second use: different array with offset */
        int val2 = array2[(key_index + 1) % ARRAY_SIZE];
        use_int(val2);
        
        /* Third use: in conditional */
        if (key_index % 3 == 0) {
            /* Use key_index again inside branch */
            double dval = darray1[key_index] + darray2[key_index];
            use_double(dval);
            
            /* More computations to increase pressure */
            int v11 = v1 + v2 + key_index;
            int v12 = v3 + v4 + key_index;
            use_int(v11);
            use_int(v12);
        } else if (key_index % 3 == 1) {
            /* Alternative path using key_index */
            int v13 = v5 + v6 + key_index;
            int v14 = v7 + v8 + key_index;
            use_int(v13);
            use_int(v14);
        } else {
            /* Third path with more complex use */
            int v15 = v9 + v10 + key_index;
            double d6 = d1 + d2 + key_index;
            use_int(v15);
            use_double(d6);
        }
        
        /* Fourth use: after conditional, in another computation */
        int combined = val1 + val2 + key_index;
        use_int(combined);
        
        /* Fifth use: pointer arithmetic */
        int* ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Switch statement splitting control flow further */
        switch (key_index % 4) {
            case 0:
                /* Use key_index in case 0 */
                darray1[key_index] += 0.1;
                break;
            case 1:
                /* Different use pattern */
                array2[key_index] += key_index;
                break;
            case 2:
                /* More uses */
                darray2[key_index] += d1;
                break;
            case 3:
                /* Complex use with multiple variables */
                array1[key_index] = v1 + v2 + v3 + key_index;
                break;
        }
        
        /* Inner loop to create cyclic data flow */
        {
            int inner_acc = 0;
            for (int inner = 0; inner < 3; inner++) {
                /* Use key_index inside inner loop */
                inner_acc += array1[(key_index + inner) % ARRAY_SIZE];
                
                /* More register pressure */
                double temp_d = d1 + d2 + d3 + inner;
                use_double(temp_d);
            }
            accumulator ^= inner_acc;
        }
        
        /* Use all variables to keep them live */
        int sum_ints = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        double sum_doubles = d1 + d2 + d3 + d4 + d5;
        
        /* Volatile operations to prevent optimization */
        volatile int vsink1 = sum_ints;
        volatile double vsink2 = sum_doubles;
        (void)vsink1;
        (void)vsink2;
        
        /* Sixth use: final computation with key_index */
        int final_val = key_index * 2 + combined;
        accumulator += final_val;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
