/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

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

__attribute__((noinline)) void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

/* Complex function to create register pressure */
__attribute__((noinline)) int compute_key(int i, int base) {
    /* Cheap but non-trivial computation - candidate for rematerialization */
    return (i * 7 + base) & 0xFFF;  /* Mask to keep values reasonable */
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory pressure */
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
    volatile double d_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 1;
        int v2 = outer * 2;
        int v3 = outer * 3;
        int v4 = outer * 4;
        int v5 = outer * 5;
        int v6 = outer * 6;
        int v7 = outer * 7;
        int v8 = outer * 8;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* Compute a key value - this is the candidate for rematerialization */
        /* It's used in multiple places, creating multiple DF_REFs */
        int key_index = compute_key(outer, 123);
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index & (ARRAY_SIZE-1)];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: conditional check */
        if (key_index > 512) {
            /* Create inner control flow with more register pressure */
            int inner1 = v1 + v2;
            int inner2 = v3 + v4;
            double inner_d = d1 + d2;
            
            /* Third use: different array indexing */
            int val2 = array2[key_index & (ARRAY_SIZE-1)];
            
            /* Use the recomputed value again */
            accumulator ^= (val1 + val2 + inner1 + inner2);
            
            /* Mixed mode computation */
            double dval1 = darray1[key_index & (ARRAY_SIZE-1)];
            d_accumulator += dval1 + inner_d;
            
            /* Call dummy function with the key */
            use_int(key_index);
        } else {
            /* Alternative path that also uses key_index */
            int alt1 = v5 + v6;
            int alt2 = v7 + v8;
            double alt_d = d3 + d4 + d5;
            
            /* Fourth use: yet another array access */
            double dval2 = darray2[key_index & (ARRAY_SIZE-1)];
            
            accumulator ^= (alt1 + alt2 - val1);
            d_accumulator += dval2 + alt_d;
            
            /* Use key_index in pointer arithmetic */
            void* ptr = &array1[key_index & (ARRAY_SIZE-1)];
            use_ptr(ptr);
        }
        
        /* Fifth use: after the conditional block */
        if ((key_index % 17) == 0) {
            /* Create a small inner loop to complicate control flow */
            int sum = 0;
            for (int j = 0; j < 3; j++) {
                sum += array1[(key_index + j) & (ARRAY_SIZE-1)];
            }
            accumulator += sum;
        }
        
        /* Use all the variables to keep them live */
        use_int(v1 + v2 + v3 + v4);
        use_double(d1 + d2 + d3 + d4 + d5);
        
        /* Sixth use: final computation with key_index */
        int final_compute = key_index * 3 + v8;
        accumulator ^= final_compute;
        
        /* Prevent loop unrolling */
        if (outer % 1000 == 0) {
            use_int(accumulator);
            use_double(d_accumulator);
        }
    }
    
    printf("Result: %d (accumulator), %f (d_accumulator)\n", 
           accumulator, d_accumulator);
    
    return 0;
}
