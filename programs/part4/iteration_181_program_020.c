/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o early-remat-trigger
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy noinline functions to prevent optimization */
__attribute__((noinline)) void use_int(volatile int x) {
    /* Empty - just to consume value */
    (void)x;
}

__attribute__((noinline)) void use_double(volatile double x) {
    /* Empty - just to consume value */
    (void)x;
}

__attribute__((noinline)) int compute_key(int i, int base) {
    /* Simple pure function - candidate for rematerialization */
    return (i * 7 + base) & 0xFF;  /* Cheap to recompute */
}

__attribute__((noinline)) double compute_scale(int i, double factor) {
    /* Another pure function with different mode (DF) */
    return (i & 0xF) * factor;
}

int main(void) {
    const int ARRAY_SIZE = 256;
    const int ITERATIONS = 100000;
    
    /* Arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double farray[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        farray[i] = i * 0.5;
    }
    
    volatile int sink = 0;  /* Volatile sink to prevent elimination */
    volatile double dsink = 0.0;
    
    /* Create high register pressure with many live variables */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Many live int variables - creates register pressure */
        int v1 = i + 1;
        int v2 = i * 2;
        int v3 = i | 0x5555;
        int v4 = i & 0xAAAA;
        int v5 = v1 + v2;
        int v6 = v3 - v4;
        int v7 = v5 ^ v6;
        int v8 = v7 * 3;
        int v9 = v8 / 2;
        int v10 = v9 % 100;
        
        /* Many live double variables - different mode */
        double d1 = i * 0.1;
        double d2 = i * 0.2;
        double d3 = i * 0.3;
        double d4 = i * 0.4;
        double d5 = d1 + d2;
        double d6 = d3 - d4;
        double d7 = d5 * d6;
        double d8 = d7 / 2.0;
        double d9 = d8 + 1.0;
        double d10 = d9 - 0.5;
        
        /* KEY COMPUTATION - cheap to recompute, used multiple times */
        /* This is the primary candidate for rematerialization */
        int key_index = compute_key(i, 123);
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        sink = val1;
        
        /* Conditional that splits control flow */
        if (key_index & 1) {
            /* Second use: different array, same index */
            int val2 = array2[key_index];
            sink = val2;
            
            /* Third use: in arithmetic */
            int val3 = val1 + val2 + key_index;
            use_int(val3);
            
            /* Inner conditional with goto to create cyclic flow */
            if (val3 > 1000) {
                goto inner_block;
            }
            
            /* Fourth use: after potential branch */
            int val4 = array1[(key_index + 1) % ARRAY_SIZE];
            sink = val4;
        } else {
            /* Alternative path with same key_index use */
            int val5 = array1[(key_index * 2) % ARRAY_SIZE];
            use_int(val5);
            
            /* Switch to create more complex control flow */
            switch (key_index % 4) {
                case 0:
                    sink = val5 + 1;
                    break;
                case 1:
                    sink = val5 + key_index;  /* Fifth use */
                    break;
                case 2:
                    sink = val5 * 2;
                    break;
                default:
                    sink = val5 / 2;
                    break;
            }
        }
        
    inner_block:
        /* Sixth use: after label, ensuring liveness across branch */
        double scale = compute_scale(key_index, 1.5);  /* Different mode! */
        
        /* Use the double result */
        double scaled_val = farray[key_index % ARRAY_SIZE] * scale;
        dsink = scaled_val;
        
        /* Seventh use: final computation with key_index */
        int final_val = (key_index + v10) & 0xFF;
        use_int(final_val);
        
        /* Consume all the variables to keep them live */
        use_int(v1); use_int(v2); use_int(v3); use_int(v4); use_int(v5);
        use_int(v6); use_int(v7); use_int(v8); use_int(v9); use_int(v10);
        use_double(d1); use_double(d2); use_double(d3); use_double(d4);
        use_double(d5); use_double(d6); use_double(d7); use_double(d8);
        use_double(d9); use_double(d10);
        
        /* Prevent loop unrolling */
        if (i % 100 == 0) {
            sink = i;
        }
    }
    
    printf("Result: sink=%d, dsink=%.2f\n", sink, dsink);
    return 0;
}
