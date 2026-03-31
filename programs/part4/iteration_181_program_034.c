/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 1000000

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
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer - 5;
        int v5 = outer / 2;
        int v6 = outer % 7;
        int v7 = outer * outer;
        int v8 = outer + 100;
        int v9 = outer - 50;
        int v10 = outer * 4;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* Key recomputable expression - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array indexing */
        int val2 = array2[key_index];
        
        /* Third use: in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            /* Use key_index again inside conditional block */
            double dval1 = darray1[key_index];
            d_accumulator += dval1;
            
            /* More computations to increase pressure */
            int v11 = v1 + key_index;
            int v12 = v2 * key_index;
            use_int(v11);
            use_int(v12);
        } else {
            /* Alternative path that also uses key_index */
            double dval2 = darray2[key_index];
            d_accumulator -= dval2;
            
            /* Different computations */
            int v13 = v3 - key_index;
            int v14 = v4 / (key_index + 1);
            use_int(v13);
            use_int(v14);
        }
        
        /* Fourth use: passed to dummy function */
        use_int(key_index);
        
        /* Fifth use: in switch statement */
        switch (key_index % 4) {
            case 0:
                accumulator += val1 + v5;
                break;
            case 1:
                accumulator += val2 + v6;
                break;
            case 2:
                accumulator += key_index + v7;
                break;
            case 3:
                accumulator += v8 - key_index;
                break;
        }
        
        /* Use double variables to create different machine modes */
        double dkey = key_index * 0.25;
        use_double(dkey);
        
        /* Complex control flow with goto to create cycles */
        if (outer % 100 == 0) {
            /* Small inner loop structure */
            int inner = 0;
            volatile int inner_sink = 0;
        inner_loop:
            if (inner < 3) {
                /* Use key_index inside inner loop-like structure */
                inner_sink += key_index + inner;
                inner++;
                goto inner_loop;
            }
            accumulator += inner_sink;
        }
        
        /* Use all the variables to keep them live */
        int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        accumulator += sum % 256;
        
        double dsum = d1 + d2 + d3 + d4 + d5;
        volatile double dsink = dsum;
        d_accumulator += dsink;
        
        /* Prevent loop invariant motion */
        if (outer % 2 == 0) {
            use_ptr(&array1[key_index % 10]);
            use_ptr(&array2[key_index % 10]);
        }
    }
    
    printf("Result: %d (accumulator), %f (d_accumulator)\n", 
           accumulator, d_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
