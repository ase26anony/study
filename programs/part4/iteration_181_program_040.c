/* early-remat-trigger.c
 * Designed to trigger early rematerialization pass in GCC RTL optimization,
 * specifically targeting the virtual register creation code in early-remat.cc
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
struct Data {
    int values[256];
    double coords[256];
    char flags[256];
};

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int *array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double *array2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    struct Data *data = (struct Data*)malloc(sizeof(struct Data));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        if (i < 256) {
            data->values[i] = i * 7;
            data->coords[i] = i * 2.5;
            data->flags[i] = i & 1;
        }
    }
    
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 12345;
        int v3 = v1 ^ v2;
        int v4 = v2 - v1;
        int v5 = v3 * 7;
        int v6 = v4 / 3;
        int v7 = v5 & 0xFF;
        int v8 = v6 | 0x7F;
        int v9 = v7 << 2;
        int v10 = v8 >> 1;
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        double d5 = d3 / 1.5;
        double d6 = d4 - d3;
        double d7 = d5 * 0.75;
        double d8 = d6 + 1.0;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Force materialization of intermediates */
        volatile int sink_v1 = v1;
        volatile double sink_d1 = d1;
        
        /* First use of key_index - array access */
        int temp1 = array1[key_index];
        
        /* Complex control flow that splits uses */
        if (outer & 1) {
            /* Second use of key_index - different mode (double) */
            double temp2 = array2[key_index];
            fp_accumulator += temp2;
            
            /* Third use - struct access with different addressing */
            if (key_index < 256) {
                int temp3 = data->values[key_index];
                accumulator ^= temp3;
                
                /* Inner conditional with another use */
                if (temp3 > 100) {
                    /* Fourth use - passed to function */
                    use_int(key_index);
                }
            }
            
            /* Small inner loop to create cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                /* Fifth use - recomputed in different context */
                int offset = (key_index + inner) % ARRAY_SIZE;
                accumulator += array1[offset];
            }
        } else {
            /* Alternative path with different uses */
            switch (key_index % 4) {
                case 0:
                    /* Sixth use - different computation */
                    accumulator += key_index * 2;
                    break;
                case 1:
                    /* Seventh use - with mode mixing */
                    fp_accumulator += (double)key_index;
                    break;
                case 2:
                    /* Eighth use - pointer arithmetic */
                    use_ptr(&array1[key_index]);
                    break;
                case 3:
                    /* Ninth use - in conditional expression */
                    accumulator += (key_index > 500) ? key_index : -key_index;
                    break;
            }
            
            /* Use double variables to ensure different modes are live */
            volatile double sink_d3 = d3;
            volatile double sink_d5 = d5;
            volatile double sink_d7 = d7;
        }
        
        /* More uses of the scalar variables to keep them live */
        v9 = use_int(v9);
        v10 = use_int(v10);
        d7 = use_double(d7);
        d8 = use_double(d8);
        
        /* Final use of key_index before loop end */
        accumulator += key_index % 64;
        
        /* Use all variables to prevent dead code elimination */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        fp_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8;
    }
    
    /* Prevent optimization of entire computation */
    printf("Result: %d (fp: %.2f)\n", accumulator, fp_accumulator);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(data);
    
    return accumulator != 0;
}
