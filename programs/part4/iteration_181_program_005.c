/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in RTL, specifically targeting the uncovered
 * code block in early-remat.cc lines 930-937.
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

/* Dummy computation to increase register pressure */
int __attribute__((noinline)) compute_pressure(int a, int b, int c, int d, 
                                               int e, int f, int g, int h) {
    return (a ^ b) + (c * d) - (e & f) + (g | h);
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    volatile int accumulator = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables - high register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer ^ 0x55;
        int v4 = outer * 3;
        int v5 = outer - 1;
        int v6 = outer | 0xAA;
        int v7 = outer & 0xFF;
        int v8 = outer / 2;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = outer * 3.3;
        double d4 = outer * 4.4;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - candidate for rematerialization */
        /* This is the key expression that should trigger remat */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int temp1 = array1[key_index];
        
        /* Second use: in conditional */
        if (key_index % 3 == 0) {
            /* Third use: different array with same index */
            double temp2 = array2[key_index];
            d1 += temp2;
            
            /* Fourth use: passed to function */
            use_int(key_index);
            
            /* Create control flow complexity */
            switch (key_index % 5) {
                case 0:
                    v1 += key_index;  /* Fifth use */
                    break;
                case 1:
                    v2 -= key_index;  /* Sixth use */
                    break;
                case 2:
                    v3 ^= key_index;  /* Seventh use */
                    break;
                case 3:
                    v4 |= key_index;  /* Eighth use */
                    break;
                default:
                    v5 &= key_index;  /* Ninth use */
                    break;
            }
        } else {
            /* Alternative path that also uses key_index */
            /* Tenth use: different computation */
            int offset = key_index % 64;
            
            /* Small inner loop to create cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                /* Eleventh use: in loop computation */
                offset += key_index + inner;
            }
            v6 += offset;
        }
        
        /* More uses of key_index after conditional */
        /* Twelfth use: in another array access */
        array1[(key_index + 1) % ARRAY_SIZE] = v7;
        
        /* Thirteenth use: in function call */
        use_ptr(&array1[key_index]);
        
        /* Force materialization with volatile */
        volatile int sink_key = key_index;
        
        /* Mixed mode computations */
        double mixed = d1 * key_index;  /* Different mode: DF from SI */
        volatile double sink_mixed = mixed;
        
        /* More register pressure with double computations */
        d2 = use_double(d2 + key_index);  /* Fourteenth use */
        d3 = use_double(d3 * key_index);  /* Fifteenth use */
        
        /* Complex expression that might need rematerialization */
        int complex_expr = (key_index << 2) | (key_index >> 3);
        
        /* Use all variables to prevent optimization */
        int pressure = compute_pressure(v1, v2, v3, v4, v5, v6, v7, v8);
        accumulator ^= pressure + complex_expr + (int)d1 + (int)d2 + (int)d3 + (int)d4;
        
        /* Another cheap recomputable expression with different mode */
        double key_double = key_index * 3.14159;
        d4 += key_double;  /* Sixteenth use - different mode */
        
        /* Use volatile to force all values to be materialized */
        volatile int sink_all = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
        volatile double sink_dall = d1 + d2 + d3 + d4;
    }
    
    printf("Result: %d\n", accumulator);
    return accumulator != 0 ? 0 : 1;
}
