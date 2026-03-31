/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
 */

#include <stdio.h>
#include <stdint.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int val) {
    volatile int sink = val;
    return sink & 1;
}

double __attribute__((noinline)) use_double(double val) {
    volatile double sink = val;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(void *ptr) {
    volatile void *sink = ptr;
    (void)sink;
}

int main(void) {
    /* Create arrays to generate memory operations */
    const int ARRAY_SIZE = 256;
    int int_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        double_array[i] = i * 1.5;
    }
    
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* High iteration count to create sustained pressure */
    const int ITERATIONS = 100000;
    
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create high register pressure with many live variables */
        int v1 = outer * 2;
        int v2 = outer + 123;
        int v3 = outer ^ 0xABCD;
        int v4 = v1 + v2;
        int v5 = v3 - v2;
        int v6 = v4 * 3;
        int v7 = v5 / 2;
        int v8 = v6 ^ v7;
        int v9 = v8 << 2;
        int v10 = v9 >> 1;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        double d5 = d3 / d4;
        double d6 = d5 - d3;
        double d7 = d4 + d6;
        double d8 = d7 * 2.0;
        double d9 = d8 / 3.0;
        double d10 = d9 - d1;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This is the expression we want to be rematerialized */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = int_array[key_index];
        
        /* Second use: in conditional */
        if (key_index > (ARRAY_SIZE / 2)) {
            /* Third use: different array with same index */
            double val2 = double_array[key_index];
            fp_accumulator += val2;
            
            /* Fourth use: in computation */
            int val3 = key_index * 3;
            v10 += val3;
        } else {
            /* Fifth use: in another computation */
            int val4 = key_index / 2;
            v9 -= val4;
            
            /* Use with pointer arithmetic */
            void *ptr = &int_array[key_index];
            use_ptr(ptr);
        }
        
        /* Sixth use: after conditional, in function call */
        int result = use_int(key_index);
        
        /* Seventh use: in another array access */
        int_array[key_index] = result + v10;
        
        /* Create inner conditional to split live ranges */
        if (outer % 3 == 0) {
            /* Eighth use: recompute in different basic block */
            int temp = key_index + v1;
            v8 += temp;
            
            /* Small inner loop to create cyclic data flow */
            for (int inner = 0; inner < 2; inner++) {
                /* Ninth use: inside inner loop */
                int inner_val = key_index + inner;
                v7 ^= inner_val;
                
                /* Force materialization */
                volatile int sink = inner_val;
                (void)sink;
            }
        } else if (outer % 3 == 1) {
            /* Tenth use: different mode (double) */
            double dtemp = (double)key_index * 1.5;
            d10 += dtemp;
            
            /* Use double version */
            double dresult = use_double(dtemp);
            fp_accumulator += dresult;
        }
        
        /* Consume all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        fp_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
        
        /* Use volatile to force writes */
        volatile int vsink = accumulator;
        volatile double fvsink = fp_accumulator;
        (void)vsink;
        (void)fvsink;
    }
    
    printf("Result: %d (fp: %f)\n", accumulator, fp_accumulator);
    return accumulator & 0xFF;
}
