/* early-remat-trigger.c
 * Designed to trigger early rematerialization pass in GCC RTL optimization
 * Specifically targets uncovered lines 930-937 in early-remat.cc
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

/* Dummy struct to create complex addressing */
struct mixed_data {
    int ints[256];
    double doubles[256];
    char bytes[1024];
};

int main(void) {
    /* Large arrays to create memory pressure */
    struct mixed_data data[4];
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < 256; i++) {
        data[0].ints[i] = i * 3;
        data[0].doubles[i] = i * 1.5;
        data[1].ints[i] = i * 5;
        data[1].doubles[i] = i * 2.5;
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < 10000; outer++) {
        /* Create many live variables to increase register pressure */
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
        int key_index = (outer * 7 + 123) % 256;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = data[0].ints[key_index];
        
        /* Second use: conditional check */
        if (key_index > 128) {
            v1 += data[1].ints[key_index];
        } else {
            v2 += data[0].ints[255 - key_index];
        }
        
        /* Third use: arithmetic operation */
        int val2 = key_index * key_index / 3;
        
        /* Fourth use: function argument */
        use_int(key_index);
        
        /* Fifth use: in a different basic block with goto */
        if (key_index % 3 == 0) {
            goto compute_block;
        } else {
            v3 += key_index;
            goto merge_point;
        }
        
    compute_block:
        /* Sixth use: in goto target block */
        v4 += key_index * 2;
        
    merge_point:
        /* Seventh use: after merge point */
        int val3 = data[0].ints[key_index] + key_index;
        
        /* Create inner loop to challenge liveness analysis */
        for (int inner = 0; inner < 3; inner++) {
            /* Eighth use: inside inner loop */
            if (inner == key_index % 3) {
                v5 += key_index + inner;
            }
            
            /* Mix with double computations for different modes */
            double dval = data[0].doubles[key_index] + inner;
            fp_accumulator += dval;
            use_double(dval);
        }
        
        /* Use key_index one more time after inner loop */
        v6 += key_index % 16;
        
        /* Complex conditional splitting uses across blocks */
        switch (key_index % 5) {
            case 0:
                v7 += key_index;
                break;
            case 1:
                v8 += key_index * 2;
                break;
            case 2:
                /* Use pointer mode computation */
                use_ptr(&data[key_index % 4].ints[key_index]);
                break;
            case 3:
                /* Double mode computation */
                d1 += data[1].doubles[key_index];
                break;
            case 4:
                /* Mixed mode */
                accumulator += key_index + (int)data[0].doubles[key_index];
                break;
        }
        
        /* Force materialization of all variables */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        accumulator += val1 + val2 + val3;
        fp_accumulator += d1 + d2 + d3 + d4;
        
        /* Use volatile to prevent dead code elimination */
        volatile int temp = accumulator;
        volatile double ftemp = fp_accumulator;
        (void)temp;
        (void)ftemp;
    }
    
    printf("Result: %d (fp: %.2f)\n", accumulator, fp_accumulator);
    return accumulator != 0 ? 0 : 1;
}
