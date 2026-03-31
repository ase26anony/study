/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * in GCC's RTL optimization phase (early-remat.cc lines 930-937)
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

/* Dummy struct to create complex addressing modes */
struct data {
    int values[256];
    double fp_values[256];
    char padding[64];
};

int main(void) {
    /* Large arrays to create memory pressure */
    struct data dataset[4];
    int array1[512];
    double array2[512];
    
    /* Initialize arrays */
    for (int i = 0; i < 512; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
    }
    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 256; j++) {
            dataset[i].values[j] = i * 1000 + j;
            dataset[i].fp_values[j] = i * 1000.0 + j * 0.5;
        }
    }
    
    /* Volatile accumulator to prevent dead code elimination */
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* High iteration count for sustained register pressure */
    const int iterations = 100000;
    
    /* Main loop with high register pressure */
    for (int i = 0; i < iterations; i++) {
        /* Create many live scalar variables to pressure registers */
        int v1 = i * 2;
        int v2 = i * 3;
        int v3 = i * 5;
        int v4 = i * 7;
        int v5 = i * 11;
        int v6 = i * 13;
        int v7 = i * 17;
        int v8 = i * 19;
        
        double d1 = i * 1.1;
        double d2 = i * 1.3;
        double d3 = i * 1.7;
        double d4 = i * 1.9;
        double d5 = i * 2.1;
        double d6 = i * 2.3;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - Candidate for rematerialization */
        /* This is the key expression that should trigger rematerialization */
        int key_index = (i * 7 + 123) % 256;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int temp1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = temp1;
        
        /* Second use: conditional check */
        if (key_index > 128) {
            /* Third use: different array with same index */
            double temp2 = array2[key_index];
            volatile double sink2 = temp2;
            fp_accumulator += sink2;
            
            /* Fourth use: struct member access */
            int temp3 = dataset[i % 4].values[key_index];
            accumulator ^= temp3;
            
            /* Complex control flow with goto to create cycles */
            if (key_index % 3 == 0) {
                goto inner_block;
            }
        } else {
            /* Fifth use: in else branch */
            double temp4 = dataset[i % 4].fp_values[key_index];
            fp_accumulator -= temp4;
        }
        
        /* Sixth use: after conditional, passed to opaque function */
        use_int(key_index);
        
        /* Inner block with label for control flow complexity */
        inner_block:
        
        /* Seventh use: in another computation */
        int offset = key_index * 2;
        if (offset < 512) {
            array1[offset] = v1 + v2;
        }
        
        /* Mixed mode computations */
        double mixed_calc = key_index * 1.5;  /* DF mode */
        use_double(mixed_calc);
        
        /* More register pressure variables */
        int v9 = v1 + v3;
        int v10 = v2 + v4;
        int v11 = v5 + v6;
        int v12 = v7 + v8;
        
        double d7 = d1 + d3;
        double d8 = d2 + d4;
        double d9 = d5 + d6;
        
        /* Use all variables to keep them live */
        accumulator += v9 + v10 + v11 + v12;
        fp_accumulator += d7 + d8 + d9;
        
        /* Pointer computation that might use key_index */
        if (i % 100 == 0) {
            void *ptr = &array1[key_index];
            use_ptr(ptr);
        }
        
        /* Switch statement for additional control flow complexity */
        switch (key_index % 5) {
            case 0:
                accumulator += 1;
                break;
            case 1:
                accumulator += key_index;  /* Eighth use */
                break;
            case 2:
                fp_accumulator += key_index * 0.25;
                break;
            case 3:
                /* Nested loop to create data flow cycles */
                for (int k = 0; k < 3; k++) {
                    accumulator += key_index + k;  /* Ninth use */
                }
                break;
            case 4:
                accumulator -= key_index;  /* Tenth use */
                break;
        }
    }
    
    /* Final result to prevent optimization */
    printf("Result: %d, %f\n", accumulator, fp_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
