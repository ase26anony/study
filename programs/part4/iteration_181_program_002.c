/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
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

/* Dummy volatile accumulator */
static volatile int global_acc = 0;
static volatile double global_acc_d = 0.0;

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = ARRAY_SIZE - i;
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = v1 ^ v2;
        int v4 = v2 * 3;
        int v5 = v3 - v4;
        int v6 = v5 & 0xFF;
        int v7 = v6 | 0x80;
        int v8 = v7 << 2;
        int v9 = v8 >> 1;
        int v10 = v9 % 17;
        
        double d1 = outer * 0.1;
        double d2 = d1 + 1.5;
        double d3 = d1 * d2;
        double d4 = d3 / 2.0;
        double d5 = d4 - 0.75;
        double d6 = d5 * 3.14159;
        double d7 = d6 / 2.71828;
        double d8 = d7 + 1.0;
        double d9 = d8 * d8;
        double d10 = d9 - d1;
        
        /* Key recomputable expression - cheap arithmetic */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Second use: in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            /* Third use: different array indexing */
            double val2 = array2[key_index];
            d10 += val2;
            
            /* Fourth use: arithmetic */
            v10 += key_index * 2;
            
            /* Complex control flow with goto to create cycles */
            if ((key_index % 3) == 0) {
                goto inner_block;
            }
        } else {
            /* Fifth use: yet another array */
            int val3 = array3[key_index];
            v10 -= val3;
        }
        
        /* Sixth use: function call argument */
        int key_mod = use_int(key_index);
        
        /* Seventh use: volatile sink */
        volatile int sink_key = key_index;
        
        /* Inner block with label for control flow complexity */
        inner_block:
        /* Eighth use: in another computation */
        int key_squared = key_index * key_index;
        v10 ^= key_squared;
        
        /* Use all the variables to keep them live */
        global_acc ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        global_acc ^= val1;
        global_acc ^= key_mod;
        
        global_acc_d += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
        
        /* More register pressure with mixed modes */
        {
            /* Force different machine modes */
            short s1 = key_index & 0xFFFF;
            char c1 = key_index & 0xFF;
            long long ll1 = (long long)key_index * 1000LL;
            float f1 = (float)key_index / 100.0f;
            
            /* Use them to prevent elimination */
            global_acc += s1 + c1;
            global_acc_d += (double)ll1 + f1;
        }
        
        /* Inner conditional that splits uses of key_index */
        switch (outer % 5) {
            case 0:
                /* Ninth use: array access with offset */
                array1[(key_index + 1) % ARRAY_SIZE] = v10;
                break;
            case 1:
                /* Tenth use: in pointer calculation */
                use_ptr(&array2[key_index % (ARRAY_SIZE/2)]);
                break;
            case 2:
                /* Eleventh use: double conversion */
                double key_as_double = (double)key_index;
                global_acc_d += use_double(key_as_double);
                break;
            case 3:
                /* Twelfth use: bit manipulation */
                key_index = (key_index << 3) | (key_index >> 5);
                global_acc ^= key_index;
                break;
            case 4:
                /* Thirteenth use: modulo operation */
                int key_mod_again = key_index % 13;
                global_acc += key_mod_again;
                break;
        }
        
        /* Final use to ensure liveness */
        if ((outer % 1000) == 0) {
            volatile int final_use = key_index;
            (void)final_use;
        }
    }
    
    printf("Result: acc=%d, acc_d=%.2f\n", global_acc, global_acc_d);
    return 0;
}
