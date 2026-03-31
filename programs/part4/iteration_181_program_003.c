/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o early-remat-trigger
 * Or with: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o early-remat-trigger
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(volatile int x) {
    /* volatile to prevent optimization */
    (void)x;
}

__attribute__((noinline)) void use_double(volatile double x) {
    /* volatile to prevent optimization */
    (void)x;
}

__attribute__((noinline)) int compute_key(int i, int base) {
    /* A simple, pure computation that's cheap to recompute */
    return (i * 7 + base) & 0xFF;  /* Mask to keep values small */
}

__attribute__((noinline)) double compute_scale(int i, double factor) {
    /* Another pure computation with different mode (DF vs SI) */
    return (i & 0xF) * factor;
}

int main(void) {
    const int ARRAY_SIZE = 256;
    const int ITERATIONS = 100000;
    
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
        darray2[i] = (ARRAY_SIZE - i) * 0.25;
    }
    
    volatile int sink = 0;
    volatile double dsink = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer;
        int v2 = outer + 1;
        int v3 = outer + 2;
        int v4 = outer + 3;
        int v5 = outer + 4;
        int v6 = outer + 5;
        int v7 = outer + 6;
        int v8 = outer + 7;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* COMPUTATION 1: Cheap to recompute integer expression */
        /* This should be a candidate for rematerialization */
        int key_index = compute_key(outer, 123);
        
        /* Use key_index in multiple places - creates multiple DF_REFs */
        int val1 = array1[key_index];      /* First use */
        sink = val1;                        /* Force materialization */
        
        /* Conditional branch that splits the uses */
        if (outer & 1) {
            /* Use key_index again in a different basic block */
            int val2 = array2[key_index];  /* Second use in different block */
            sink = val2;
            
            /* More computations to increase pressure */
            d1 += darray1[key_index];
            dsink = d1;
        } else {
            /* Alternative path also using key_index */
            double scale = compute_scale(key_index, 1.5);
            dsink = scale;
            
            /* More variables to increase pressure */
            int tmp = v1 + v2 + v3;
            sink = tmp;
        }
        
        /* Third use of key_index after the conditional */
        double dval = darray2[key_index];
        dsink = dval;
        
        /* Use the recomputed value in a switch statement */
        switch (key_index & 0x3) {  /* Switch on low bits of key_index */
            case 0:
                v1 = array1[key_index >> 1];
                sink = v1;
                break;
            case 1:
                v2 = array2[key_index >> 1];
                sink = v2;
                break;
            case 2:
                /* Inner loop to create cyclic data flow */
                for (int inner = 0; inner < 3; inner++) {
                    /* Use key_index inside inner loop */
                    int inner_val = key_index + inner;
                    sink = inner_val;
                }
                break;
            case 3:
                /* Use goto to create additional control flow */
                if (key_index > 128) {
                    goto special_case;
                }
                v3 = key_index * 2;
                sink = v3;
                break;
        }
        
        /* COMPUTATION 2: Another recomputable value with different mode */
        double scale_factor = compute_scale(outer, 2.0);
        
        /* Use scale_factor in multiple places */
        d2 = darray1[outer & 0xFF] * scale_factor;
        dsink = d2;
        
        if (outer & 2) {
            d3 = darray2[outer & 0xFF] / scale_factor;
            dsink = d3;
        }
        
        /* More register pressure with mixed types */
        int sum1 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        double sum2 = d1 + d2 + d3 + d4 + d5;
        
        /* Use volatile operations to force materialization */
        volatile int vol_int = sum1;
        volatile double vol_double = sum2;
        
        /* Consume results to prevent elimination */
        use_int(vol_int);
        use_double(vol_double);
        
        continue;  /* Skip the label in normal flow */
        
    special_case:
        /* Handle the goto case */
        v4 = key_index * 3;  /* Use key_index again */
        sink = v4;
    }
    
    printf("Result: sink = %d, dsink = %f\n", sink, dsink);
    return 0;
}
