/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create new virtual registers
 * and execute the uncovered validation code in early-remat.cc lines 930-937
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
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

/* Dummy computation to create register pressure */
int __attribute__((noinline)) compute_offset(int i, int base) {
    /* Complex enough to not be optimized away, simple enough to rematerialize */
    return (i * 7 + 123) ^ (base & 0xFF);
}

double __attribute__((noinline)) compute_scale(int i, double factor) {
    /* Mixed mode computation */
    return (i & 0xF) * factor + 1.5;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
    }
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer;
        int v2 = outer * 2;
        int v3 = outer + 1;
        int v4 = outer ^ 0x55AA;
        int v5 = outer % 17;
        int v6 = outer << 2;
        int v7 = outer >> 1;
        int v8 = ~outer;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        
        /* KEY COMPUTATION: This should be a rematerialization candidate */
        /* It's used in multiple places with complex control flow */
        int key_index = compute_offset(outer, v1);
        
        /* Use key_index in multiple distinct operations */
        
        /* 1. Array access in SI mode */
        int val1 = array1[key_index % ARRAY_SIZE];
        accumulator ^= val1;
        
        /* 2. Another computation using key_index */
        int offset = key_index + v2;
        if (offset & 1) {
            /* Conditional block that uses key_index again */
            int adjusted = key_index * 3;
            use_int(adjusted);
            
            /* Inner conditional with goto to create complex CFG */
            if (adjusted > 1000) {
                goto special_case;
            }
            
            /* Use in array index again */
            double temp = array2[(key_index + 5) % ARRAY_SIZE];
            d_accumulator += temp;
        }
        
        /* 3. Mixed mode computation using key_index */
        double scale_factor = compute_scale(key_index, d1);
        d_accumulator *= scale_factor;
        
        /* 4. Pass to opaque function */
        use_int(key_index);
        
        /* More register pressure variables */
        int v9 = v1 + v2;
        int v10 = v3 * v4;
        int v11 = v5 | v6;
        int v12 = v7 & v8;
        
        double d5 = d1 + d2;
        double d6 = d3 * d4;
        double d7 = d5 - d6;
        
        /* Use all variables to prevent optimization */
        accumulator += v9 + v10 + v11 + v12;
        d_accumulator += d5 + d6 + d7;
        
        /* Complex control flow continues */
        switch (key_index % 5) {
            case 0:
                /* Use key_index in another computation */
                use_int(key_index * 2);
                break;
            case 1:
                /* Different use pattern */
                use_double(key_index * 0.25);
                break;
            case 2:
                /* Create pointer computation */
                use_ptr(&array1[key_index % ARRAY_SIZE]);
                break;
            default:
                /* More uses */
                accumulator ^= key_index;
                break;
        }
        
        /* Continue after label */
        continue;
        
    special_case:
        /* Alternative path that also uses key_index */
        int special_val = key_index % 256;
        accumulator |= special_val;
        /* Loop back */
        if (outer & 1) {
            /* Another conditional use */
            use_int(key_index + 1000);
        }
    }
    
    /* Final result to prevent complete optimization */
    printf("Result: %d (%.2f)\n", accumulator, d_accumulator);
    return accumulator != 0 ? 0 : 1;
}
