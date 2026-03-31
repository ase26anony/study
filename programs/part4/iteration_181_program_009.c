/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
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

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory operations */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = ARRAY_SIZE - i;
    }
    
    volatile int global_sink = 0;
    volatile double dbl_sink = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 3;
        int v2 = outer + 17;
        int v3 = outer ^ 0xABCD;
        int v4 = outer % 31;
        int v5 = outer << 2;
        int v6 = outer >> 1;
        int v7 = outer | 0xFF;
        int v8 = outer & 0x7F;
        int v9 = outer + v1;
        int v10 = v2 - v3;
        
        double d1 = outer * 0.3;
        double d2 = outer * 0.7;
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        double d5 = d3 / 2.0;
        
        /* COMPUTATION TO BE REMATERIALIZED - cheap to recompute */
        /* This creates a value in SI mode (int) */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index multiple times in different contexts */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Second use: in arithmetic expression */
        int val2 = array3[key_index] + key_index;
        
        /* Third use: conditional check */
        if (key_index > ARRAY_SIZE / 2) {
            /* Use key_index again inside branch */
            double temp = array2[key_index];
            dbl_sink += temp;
            
            /* More variables to increase pressure in this block */
            int v11 = v1 + key_index;
            int v12 = v2 * key_index;
            use_int(v11);
            use_int(v12);
        } else {
            /* Alternative path also uses key_index */
            int v13 = v3 - key_index;
            int v14 = v4 ^ key_index;
            use_int(v13);
            use_int(v14);
        }
        
        /* Fourth use: function call argument */
        use_int(key_index);
        
        /* Fifth use: pointer calculation */
        void *ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Create another recomputable value in DF mode (double) */
        double recomputed_dbl = (outer * 3.14159) / 1024.0;
        
        /* Multiple uses of the double value */
        dbl_sink += recomputed_dbl;
        double scaled = recomputed_dbl * 2.0;
        use_double(scaled);
        
        if (outer % 3 == 0) {
            dbl_sink += recomputed_dbl * 3.0;
        }
        
        /* Complex control flow with goto to create cycles */
        if (outer % 100 == 0) {
            int counter = 0;
        inner_loop:
            /* Small inner loop using key_index */
            int inner_val = key_index + counter;
            use_int(inner_val);
            
            counter++;
            if (counter < 3) {
                goto inner_loop;  /* Manual inner loop */
            }
        }
        
        /* Switch statement splitting uses of recomputed values */
        switch (outer % 4) {
            case 0:
                global_sink += val1 + key_index;
                break;
            case 1:
                global_sink += val2 - key_index;
                break;
            case 2:
                global_sink += key_index * 2;
                break;
            case 3:
                global_sink += key_index ^ v5;
                break;
        }
        
        /* Consume all variables to prevent dead code elimination */
        global_sink += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        dbl_sink += d1 + d2 + d3 + d4 + d5;
        
        /* Volatile stores to force materialization */
        volatile int force_store = key_index;
        (void)force_store;
    }
    
    printf("Result: %d (sink: %f)\n", global_sink, dbl_sink);
    return global_sink != 0 ? 0 : 1;
}
