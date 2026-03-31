/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink + 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 1.01;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
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
    
    volatile int accumulator = 0;
    
    /* High-pressure loop with recomputable values */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 12345;
        int v3 = outer ^ 0xABCD;
        int v4 = outer % 17;
        int v5 = outer << 3;
        int v6 = outer >> 2;
        int v7 = outer | 0xFF;
        int v8 = outer & 0x7F;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        
        /* Key recomputable expression - cheap to recompute */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple places (creates multiple DF_REFs) */
        int val1 = array1[key_index];      /* First use */
        double val2 = array2[key_index];   /* Second use, different mode */
        
        /* Consume variables to prevent optimization */
        v1 = use_int(v1);
        d1 = use_double(d1);
        
        /* Complex control flow splitting uses of key_index */
        if (outer % 3 == 0) {
            /* Use key_index again in conditional block */
            int val3 = array3[key_index];  /* Third use */
            accumulator ^= val3;
            
            /* More variables to increase pressure */
            int v9 = v2 + v3;
            int v10 = v4 * v5;
            double d5 = d2 + d3;
            
            v9 = use_int(v9);
            d5 = use_double(d5);
            
            /* Inner conditional creating more data flow */
            if (key_index % 2 == 0) {      /* Fourth use of key_index */
                accumulator += val1;
                use_ptr(&array1[key_index]); /* Fifth use */
            } else {
                accumulator -= val1;
            }
        } else if (outer % 3 == 1) {
            /* Alternative path with different key_index usage */
            int temp = key_index * 2;      /* Sixth use */
            accumulator |= temp;
            
            /* More computations to maintain pressure */
            double d6 = d3 * d4;
            int v11 = v6 + v7;
            
            d6 = use_double(d6);
            v11 = use_int(v11);
        } else {
            /* Third path with switch statement */
            switch (key_index % 4) {       /* Seventh use */
                case 0:
                    accumulator += key_index; /* Eighth use */
                    break;
                case 1:
                    accumulator -= key_index; /* Ninth use */
                    break;
                default:
                    accumulator ^= key_index; /* Tenth use */
                    break;
            }
        }
        
        /* Use key_index one more time before loop ends */
        if (val2 > 100.0) {                /* Uses val2 from key_index */
            accumulator += key_index % 16; /* Eleventh use */
        }
        
        /* Consume remaining variables */
        v2 = use_int(v2);
        v3 = use_int(v3);
        v4 = use_int(v4);
        d2 = use_double(d2);
        d3 = use_double(d3);
        
        /* Force materialization of results */
        volatile int sink1 = v5;
        volatile int sink2 = v6;
        volatile double sink3 = d4;
        (void)sink1; (void)sink2; (void)sink3;
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
