/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy noinline functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink ^ 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(int *p) {
    volatile int sink = *p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory references */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 5;
        darray[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    
    /* High register pressure loop */
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
        
        double d1 = outer * 1.1;
        double d2 = outer * 1.3;
        double d3 = outer * 1.7;
        double d4 = outer * 1.9;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        
        /* Second use: conditional check */
        if (key_index > ARRAY_SIZE / 2) {
            /* Use key_index again in this branch */
            int val2 = array2[key_index];
            v1 += val2;
            
            /* More computations to increase pressure */
            d1 += darray[key_index];
            
            /* Use dummy function */
            v2 = use_int(key_index);
        } else {
            /* Alternative use in else branch */
            d2 = use_double(key_index * 1.0);
            
            /* Complex expression using key_index */
            v3 = (key_index * v4) / (v5 + 1);
        }
        
        /* Third use: in a switch with multiple cases */
        switch (key_index % 4) {
            case 0:
                v6 = key_index + v7;
                break;
            case 1:
                v7 = key_index - v8;
                break;
            case 2:
                v8 = key_index * v1;
                break;
            case 3:
                v1 = key_index / (v2 + 1);
                break;
        }
        
        /* Fourth use: pointer arithmetic and function call */
        use_ptr(&array1[key_index]);
        
        /* Fifth use: in another conditional with goto (creates CFG complexity) */
        if (key_index % 3 == 0) {
            v4 = key_index * 2;
            /* Small inner loop to create cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                v5 += key_index + inner;
            }
        } else if (key_index % 3 == 1) {
            v5 = key_index * 3;
            /* Use goto to create additional control flow edge */
            if (v5 > 1000) goto special_case;
        } else {
            v6 = key_index * 4;
        }
        
        /* Continue normal flow */
        v7 = v1 + v2 + v3;
        
        /* Label for goto target */
        special_case:
        v8 = v4 + v5 + v6;
        
        /* Mixed mode computations */
        double d5 = d1 + d2 + key_index;
        double d6 = d3 * d4 - key_index;
        
        /* Use all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        accumulator += (int)(d1 + d2 + d3 + d4 + d5 + d6);
        
        /* Additional cheap recomputable value with different mode (double) */
        double recompute_dbl = (outer * 1.2345) + 67.89;
        
        /* Use double value multiple times */
        d1 += recompute_dbl;
        d2 -= recompute_dbl;
        if (recompute_dbl > 100.0) {
            d3 *= recompute_dbl;
        }
        
        /* Force double computation to have side effect */
        volatile double dsink = recompute_dbl;
        (void)dsink;
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0;
}
