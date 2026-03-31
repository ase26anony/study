/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
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

__attribute__((noinline)) void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

/* Complex control flow helper */
__attribute__((noinline)) int conditional_transform(int x, int y) {
    if (x > y) return x * 2 - y;
    if (x < y) return y * 3 - x;
    return x + y;
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
        darray2[i] = (ARRAY_SIZE - i) * 0.25;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create high register pressure with many live variables */
        int v1 = outer * 3;
        int v2 = outer + 17;
        int v3 = outer / 2;
        int v4 = outer % 13;
        int v5 = v1 + v2;
        int v6 = v3 - v4;
        int v7 = v5 * v6;
        int v8 = v7 & 0xFF;
        int v9 = v8 | 0x1F;
        int v10 = v9 ^ v1;
        
        double d1 = outer * 0.3;
        double d2 = outer * 0.7;
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        double d5 = d3 / (d4 + 1.0);
        double d6 = d5 - d1;
        double d7 = d6 * 2.0;
        double d8 = d7 + 3.14;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        int val2 = array2[key_index];
        
        /* Second use: in conditional */
        if (key_index % 3 == 0) {
            v10 += val1;
            d8 += darray1[key_index];
        } else if (key_index % 3 == 1) {
            v10 -= val2;
            d8 -= darray2[key_index];
        } else {
            /* Third use: in arithmetic */
            v10 += key_index * 2;
            d8 += key_index * 0.5;
        }
        
        /* Fourth use: passed to function */
        use_int(key_index);
        
        /* Complex control flow with inner conditional */
        switch (key_index % 5) {
            case 0:
                /* Fifth use: different mode (pointer arithmetic) */
                use_ptr(&array1[key_index]);
                v10 = conditional_transform(v10, key_index);
                break;
            case 1:
                /* Use in floating point context */
                d8 += key_index;  // implicit conversion
                v10 = key_index / (v1 + 1);
                break;
            case 2:
                /* Create cyclic data flow with goto */
                {
                    int temp = key_index;
                retry_label:
                    temp = (temp * 3 + 1) % 100;
                    if (temp < 50) {
                        v10 += temp;
                        goto retry_label;
                    }
                    v10 += key_index;  // Sixth use
                }
                break;
            case 3:
                /* Use in both int and double contexts */
                v10 += key_index * key_index;
                d8 += key_index * 0.01;
                break;
            default:
                /* Seventh use: in another array access */
                d_accumulator += darray1[key_index] - darray2[key_index];
                break;
        }
        
        /* Force materialization with volatile */
        volatile int sink_int = key_index;  // Eighth use
        (void)sink_int;
        
        /* More register pressure variables */
        int v11 = v10 + key_index;  // Ninth use
        int v12 = v11 * 2;
        int v13 = v12 / 3;
        int v14 = v13 % 7;
        int v15 = v14 ^ v10;
        
        double d9 = d8 + key_index;  // Tenth use (different mode)
        double d10 = d9 * 1.1;
        double d11 = d10 / 2.0;
        double d12 = d11 - 0.5;
        
        /* Consume all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        accumulator ^= v11 ^ v12 ^ v13 ^ v14 ^ v15;
        
        d_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
        d_accumulator += d9 + d10 + d11 + d12;
        
        /* Use functions to prevent dead code elimination */
        use_int(v15);
        use_double(d12);
        
        /* Inner loop to create more complex control flow */
        for (int inner = 0; inner < 3; inner++) {
            /* Recompute key_index again - should trigger remat analysis */
            int local_key = (key_index + inner) % ARRAY_SIZE;  // Uses key_index
            accumulator += array1[local_key];
            d_accumulator += darray1[local_key];
        }
    }
    
    printf("Result: %d (int), %f (double)\n", accumulator, d_accumulator);
    return 0;
}
