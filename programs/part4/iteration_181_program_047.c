/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers for recomputable values under high register pressure.
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
int __attribute__((noinline)) use_int(int x) {
    volatile int sink = x;
    return sink ^ 1;
}

double __attribute__((noinline)) use_double(double x) {
    volatile double sink = x;
    return sink * 0.5;
}

void __attribute__((noinline)) use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Dummy computation to create recomputable expressions */
int __attribute__((noinline)) compute_key(int i, int base) {
    /* Cheap, pure computation - candidate for rematerialization */
    return (i * 7 + base) & 0xFFF;
}

double __attribute__((noinline)) compute_scale(int i) {
    /* Another cheap computation in different mode (DF) */
    return (i & 0xF) * 0.1;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory references */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    volatile int *volatile ptr_array[ARRAY_SIZE/16];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i ^ 0x1234;
        array2[i] = i * 0.01;
        if (i % 16 == 0) {
            ptr_array[i/16] = (volatile int*)&array1[i];
        }
    }
    
    volatile int accumulator = 0;
    volatile double fp_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create high register pressure with many live variables */
        int v1 = outer ^ 0x1111;
        int v2 = outer * 3;
        int v3 = outer + 0x2222;
        int v4 = outer & 0xFF;
        int v5 = v1 + v2;
        int v6 = v3 - v4;
        
        double d1 = outer * 0.01;
        double d2 = outer * 0.02;
        double d3 = d1 + d2;
        double d4 = d1 * d2;
        
        /* Recomputation candidate 1: cheap int expression */
        int key_index = compute_key(outer, 123);
        /* This creates a DF_REF to key_index */
        
        /* Use key_index in multiple distinct places */
        int temp1 = array1[key_index & (ARRAY_SIZE-1)];
        
        /* Volatile use to force materialization */
        volatile int sink1 = key_index;
        
        /* Complex control flow splitting uses */
        if (outer & 0x1) {
            /* Use key_index again in different basic block */
            int temp2 = array1[(key_index + 1) & (ARRAY_SIZE-1)];
            v1 += temp2;
            
            /* Inner conditional creating more data flow */
            if (outer & 0x2) {
                /* Another use of key_index */
                use_int(key_index * 2);
                v2 ^= key_index;
            } else {
                /* Different path still using key_index */
                v3 += key_index;
                goto label_within_loop;  /* Create cyclic flow */
            }
            
            /* Use in pointer calculation */
            if (key_index < ARRAY_SIZE/16) {
                use_ptr((void*)ptr_array[key_index]);
            }
        } else {
            /* Alternative path with different key_index usage */
            double scale = compute_scale(key_index);
            d3 += scale;
            fp_accumulator += d3;
        }
        
    label_within_loop:
        /* Recomputation candidate 2: cheap double expression */
        double scale_factor = compute_scale(outer);
        
        /* Use in multiple places with different modes */
        d4 = array2[outer & (ARRAY_SIZE-1)] * scale_factor;
        fp_accumulator += d4;
        
        /* More register pressure variables */
        int v7 = v5 + v6;
        int v8 = v7 * 2;
        double d5 = d3 + d4;
        double d6 = d5 * scale_factor;
        
        /* Use key_index again after control flow merge */
        if (key_index > 100) {
            v8 += key_index;
        }
        
        /* Force all values to be used */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        accumulator += temp1;
        
        /* Volatile operations prevent elimination */
        volatile double sink2 = d6;
        fp_accumulator += sink2;
        
        /* Small inner loop to create complex data flow */
        for (int inner = 0; inner < 2; inner++) {
            /* Use key_index inside inner loop */
            int inner_temp = key_index + inner;
            accumulator ^= inner_temp;
            
            /* More register pressure */
            double inner_d = d1 + inner * 0.1;
            fp_accumulator += inner_d;
        }
        
        /* Switch statement splitting key_index uses */
        switch (outer & 0x3) {
            case 0:
                use_int(key_index + v1);
                break;
            case 1:
                v2 += key_index;
                break;
            case 2:
                use_double(key_index * 0.01);
                break;
            case 3:
                /* Fall through to default */
            default:
                accumulator += key_index;
                break;
        }
    }
    
    printf("Result: %d (fp: %f)\n", accumulator, fp_accumulator);
    return accumulator != 0 ? 0 : 1;
}
