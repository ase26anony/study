/* early-remat-trigger.c
 * Designed to trigger early rematerialization pass in GCC RTL optimizer
 * Specifically targets creation of new virtual registers in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink & 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Dummy struct to create more register pressure */
struct Data {
    int a, b, c;
    double x, y, z;
};

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int arr1[ARRAY_SIZE];
    double arr2[ARRAY_SIZE];
    struct Data arr3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        arr2[i] = i * 0.5;
        arr3[i].a = i;
        arr3[i].b = i * 2;
        arr3[i].c = i * 3;
        arr3[i].x = i * 0.1;
        arr3[i].y = i * 0.2;
        arr3[i].z = i * 0.3;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to pressure registers */
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
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = arr1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: different array with different mode (double) */
        double val2 = arr2[key_index];
        
        /* Third use: struct member access */
        int val3 = arr3[key_index].a + arr3[key_index].b;
        
        /* Use in conditional - creates control flow complexity */
        if (key_index % 3 == 0) {
            /* Inner block with more computations */
            int inner_val = key_index * 2;
            accumulator += inner_val;
            
            /* Use key_index again in this block */
            d_accumulator += arr2[key_index] * 0.5;
            
            /* More register pressure variables */
            int t1 = v1 + v2;
            int t2 = v3 + v4;
            double dt1 = d1 + d2;
            double dt2 = d3 + d4;
            
            /* Use all variables to prevent optimization */
            accumulator ^= t1 ^ t2;
            d_accumulator += dt1 + dt2;
        } else if (key_index % 3 == 1) {
            /* Alternative path with different uses */
            int alt_val = key_index * 3;
            accumulator -= alt_val;
            
            /* Use key_index with pointer arithmetic */
            void *ptr = &arr3[key_index];
            use_ptr(ptr);
            
            /* More computations to maintain pressure */
            double dt3 = d2 * d3;
            int t3 = v5 * v6;
            d_accumulator -= dt3;
            accumulator |= t3;
        } else {
            /* Third path - creates more control flow complexity */
            /* Use key_index in a loop-like construct */
            int temp = key_index;
            for (int j = 0; j < 3; j++) {
                temp = (temp * 13 + 7) % 256;
                accumulator ^= temp;
            }
            
            /* Use key_index after the mini-loop */
            d_accumulator += arr2[key_index];
        }
        
        /* Fourth use: pass to opaque function */
        int func_result = use_int(key_index);
        
        /* Fifth use: in another array computation */
        int idx2 = (key_index + 1) % ARRAY_SIZE;
        int val4 = arr1[idx2] + arr1[key_index];
        
        /* Use all the computed values to prevent elimination */
        accumulator += val1 + val3 + val4 + func_result;
        d_accumulator += val2;
        
        /* Use all the register pressure variables */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        d_accumulator += d1 + d2 + d3 + d4 + d5;
        
        /* Create a situation where key_index might need rematerialization */
        /* by using it in a complex expression after other computations */
        if (outer % 100 == 0) {
            /* Periodic use that might force different register allocation */
            int special = key_index * key_index;
            accumulator += special;
            
            /* Mixed mode computation */
            double mixed = key_index * 0.25;
            d_accumulator += mixed;
        }
    }
    
    /* Final result to prevent entire loop from being optimized away */
    printf("Result: %d, %f\n", accumulator, d_accumulator);
    
    return 0;
}
