/* early-remat-trigger.c */
/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o early-remat-trigger */

#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(volatile int x) {
    (void)x; /* Prevent unused parameter warning */
}

__attribute__((noinline)) void use_double(volatile double x) {
    (void)x;
}

__attribute__((noinline)) int compute_offset(int i, int base) {
    return (i * 7 + base) & 0xFF; /* Cheap, pure computation */
}

__attribute__((noinline)) double compute_scale(int i, double factor) {
    return (i & 0xF) * factor; /* Cheap FP computation */
}

int main(void) {
    const int ARRAY_SIZE = 256;
    const int ITERATIONS = 100000;
    
    /* Arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double farray1[ARRAY_SIZE];
    double farray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        farray1[i] = i * 0.5;
        farray2[i] = i * 1.5;
    }
    
    volatile int sink_int = 0;
    volatile double sink_double = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create register pressure with many live variables */
        int v1 = outer;
        int v2 = outer + 1;
        int v3 = outer + 2;
        int v4 = outer + 3;
        int v5 = outer + 4;
        int v6 = outer + 5;
        int v7 = outer + 6;
        int v8 = outer + 7;
        int v9 = outer + 8;
        int v10 = outer + 9;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* CHEAP RECOMPUTABLE VALUE - candidate for rematerialization */
        int key_index = compute_offset(outer, 123);
        
        /* Use key_index multiple times in different contexts */
        int idx1 = key_index % ARRAY_SIZE;
        int idx2 = (key_index * 3) % ARRAY_SIZE;
        
        /* First use - array indexing */
        int val1 = array1[idx1] + array2[idx2];
        sink_int = val1; /* Force materialization */
        
        /* Second use - conditional branch */
        if (key_index > 128) {
            /* Use in different basic block */
            int val2 = array2[idx1] - array1[idx2];
            sink_int = val2;
            
            /* Create inner control flow complexity */
            for (int inner = 0; inner < 3; inner++) {
                /* Use key_index again inside inner loop */
                int temp = key_index + inner;
                use_int(temp);
            }
            
            /* Use with mixed types */
            double scale = compute_scale(key_index, 2.5);
            sink_double = scale;
            
            /* More register pressure */
            double d6 = d1 * d2;
            double d7 = d3 * d4;
            sink_double = d6 + d7;
        } else {
            /* Alternative path using key_index */
            int val3 = key_index * key_index;
            sink_int = val3;
            
            /* FP computation with different mode */
            double offset = compute_scale(key_index, 0.75);
            sink_double = offset;
        }
        
        /* Third use - after conditional */
        int idx3 = (key_index + 5) % ARRAY_SIZE;
        double fval = farray1[idx3] * farray2[idx1];
        sink_double = fval;
        
        /* Fourth use - function call */
        use_int(key_index);
        
        /* More register pressure to ensure spilling/remat consideration */
        int v11 = v1 + v2 + v3;
        int v12 = v4 + v5 + v6;
        int v13 = v7 + v8 + v9;
        int v14 = v10 + v11 + v12;
        sink_int = v13 + v14;
        
        double d8 = d1 + d2 + d3;
        double d9 = d4 + d5 + d8;
        sink_double = d9;
        
        /* Complex expression using key_index again */
        int complex = (key_index * v1) + (key_index / (v2 + 1));
        use_int(complex);
        
        /* Switch statement to create more control flow */
        switch (key_index % 4) {
            case 0:
                sink_int = array1[key_index % ARRAY_SIZE];
                break;
            case 1:
                sink_int = array2[key_index % ARRAY_SIZE];
                break;
            case 2:
                sink_double = farray1[key_index % ARRAY_SIZE];
                break;
            case 3:
                sink_double = farray2[key_index % ARRAY_SIZE];
                break;
        }
        
        /* Prevent loop invariant motion */
        if (outer % 1000 == 0) {
            sink_int = outer;
        }
    }
    
    printf("Result: sink_int = %d, sink_double = %f\n", sink_int, sink_double);
    return 0;
}
