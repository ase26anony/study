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
    const int ITERS = 100000;
    
    /* Arrays to create memory references */
    int int_array[ARRAY_SIZE];
    double double_array[ARRAY_SIZE];
    void *ptr_array[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        int_array[i] = i * 3;
        double_array[i] = i * 1.5;
        ptr_array[i] = &int_array[i];
    }
    
    volatile int global_sink = 0;
    volatile double fp_sink = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERS; outer++) {
        /* Create high register pressure with many live variables */
        int v1 = outer * 2;
        int v2 = outer + 123;
        int v3 = outer ^ 0xABCD;
        int v4 = v1 + v2;
        int v5 = v3 - v2;
        int v6 = v4 * v5;
        int v7 = v6 / (v1 + 1);
        int v8 = v7 & 0xFF;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = d1 + d2;
        double d4 = d2 - d1;
        double d5 = d3 * d4;
        double d6 = d5 / (outer + 1.0);
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute, used multiple times */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* First use of key_index - array access */
        int val1 = int_array[key_index];
        
        /* Complex control flow splitting uses */
        if (outer % 3 == 0) {
            /* Second use in conditional block */
            double val2 = double_array[key_index];
            fp_sink += val2;
            
            /* More register pressure inside branch */
            int t1 = v8 + key_index;
            int t2 = t1 * 2;
            int t3 = t2 - key_index;
            global_sink ^= t3;
            
            /* Inner loop to create cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                /* Third use inside inner loop */
                void *ptr = ptr_array[key_index];
                use_ptr(ptr);
                
                /* More computations to maintain pressure */
                int inner_val = (key_index + inner) % ARRAY_SIZE;
                global_sink += int_array[inner_val];
            }
        } else if (outer % 3 == 1) {
            /* Alternative path with different mode usage */
            /* Fourth use - passed to function */
            use_int(key_index);
            
            /* Mixed mode computations */
            double d7 = key_index * 2.5;
            fp_sink += d7;
            
            /* Create data flow with goto to challenge analysis */
            if (key_index % 2 == 0) {
                goto compute_path;
            }
            
            int t4 = key_index * 3;
            global_sink += t4;
            continue;
            
        compute_path:
            /* Fifth use after label */
            int t5 = key_index + 1000;
            global_sink ^= t5;
        } else {
            /* Third path with switch statement */
            switch (key_index % 4) {
                case 0:
                    /* Sixth use in switch case */
                    double_array[key_index] += 1.0;
                    break;
                case 1:
                    /* Seventh use */
                    int_array[key_index] += key_index;
                    break;
                case 2:
                    /* Eighth use */
                    fp_sink += use_double(key_index);
                    break;
                default:
                    /* Ninth use */
                    global_sink += key_index * 2;
                    break;
            }
        }
        
        /* Final use outside conditionals - ensures value is live across branches */
        /* Tenth use */
        int final_val = int_array[key_index] + key_index;
        global_sink += final_val;
        
        /* Consume all variables to prevent optimization */
        global_sink ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        fp_sink += d1 + d2 + d3 + d4 + d5 + d6;
        
        /* Additional volatile operations to force materialization */
        volatile int force_materialize = key_index;
        (void)force_materialize;
    }
    
    printf("Result: %d (%.2f)\n", global_sink, fp_sink);
    return global_sink != 0 ? 0 : 1;
}
