/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(volatile int x) {
    (void)x;
}

__attribute__((noinline)) void use_double(volatile double x) {
    (void)x;
}

__attribute__((noinline)) int compute_mask(int a, int b) {
    return (a ^ b) & 0xFF;
}

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 1000000

int main(void) {
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    volatile int sink = 0;
    volatile double dsink = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = v1 ^ v2;
        int v4 = v2 * 3;
        int v5 = v3 + v4;
        int v6 = v5 % 17;
        int v7 = v6 << 2;
        int v8 = v7 | 0x0F;
        
        double d1 = outer * 0.25;
        double d2 = d1 + 1.5;
        double d3 = d1 * d2;
        double d4 = d3 / 2.0;
        double d5 = d4 - 0.75;
        double d6 = d5 * 3.14159;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        sink = val1;
        
        /* Second use: in conditional with complex control flow */
        if (key_index % 3 == 0) {
            /* Inner conditional block creates separate basic block */
            int val2 = array1[(key_index + 1) % ARRAY_SIZE];
            v8 += val2;
            
            /* Use key_index again in this block */
            if (key_index > ARRAY_SIZE / 2) {
                double dval = array2[key_index];
                dsink = dval;
                v7 = (int)dval;
            }
            
            /* Small inner loop to create cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                /* Use key_index inside inner loop */
                int temp = key_index + inner;
                v6 ^= temp;
            }
        } else if (key_index % 3 == 1) {
            /* Alternative path that also uses key_index */
            double dval = array2[key_index];
            dsink = dval * 2.0;
            v5 = (int)dval;
        } else {
            /* Third path with switch statement */
            switch (key_index % 5) {
                case 0: v4 = key_index * 2; break;
                case 1: v3 = key_index | 0xFF; break;
                case 2: v2 = key_index ^ 0xAA; break;
                case 3: v1 = key_index + 100; break;
                default: v8 = key_index - 50; break;
            }
        }
        
        /* Third use: function call argument */
        use_int(key_index);
        
        /* Fourth use: in another computation */
        int mask = compute_mask(key_index, v8);
        
        /* Fifth use: array indexing with different mode (double) */
        double dval2 = array2[key_index % (ARRAY_SIZE / 2)];
        dsink = dval2;
        
        /* Mix computations with different modes */
        if (key_index % 7 == 0) {
            /* Create double mode computation */
            double dkey = (double)key_index;
            double dcomp = dkey * 1.618 + dval2;
            use_double(dcomp);
            
            /* Use in integer context too */
            v1 = (int)dcomp;
        }
        
        /* More register pressure variables */
        int v9 = v1 + v2;
        int v10 = v3 * v4;
        int v11 = v5 ^ v6;
        int v12 = v7 | v8;
        int v13 = v9 - v10;
        int v14 = v11 & v12;
        int v15 = v13 + v14;
        
        double d7 = d1 + d2;
        double d8 = d3 - d4;
        double d9 = d5 * d6;
        double d10 = d7 / d8;
        double d11 = d9 + d10;
        
        /* Consume all variables to prevent optimization */
        sink = v15 + mask;
        dsink = d11;
        
        /* Use goto to create additional control flow complexity */
        if (outer % 100 == 0) {
            goto special_case;
        }
        
        continue;
        
    special_case:
        /* Use key_index again in goto target block */
        int special_val = array1[(key_index * 2) % ARRAY_SIZE];
        sink = special_val;
    }
    
    printf("Result: sink=%d, dsink=%f\n", sink, dsink);
    return 0;
}
