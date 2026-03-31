/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass
 * to execute the virtual register creation code in early-remat.cc
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
__attribute__((noinline)) void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

__attribute__((noinline)) void use_ptr(void *p) {
    volatile void *sink = p;
    (void)sink;
}

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = i * 7;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main loop with high register pressure */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to pressure registers */
        int v1 = outer * 2;
        int v2 = outer + 123;
        int v3 = outer ^ 0x55AA;
        int v4 = outer % 17;
        int v5 = v1 + v2;
        int v6 = v3 - v4;
        int v7 = v5 * v6;
        int v8 = v7 / (v4 + 1);
        
        double d1 = outer * 1.1;
        double d2 = outer * 2.2;
        double d3 = outer * 3.3;
        double d4 = d1 + d2;
        double d5 = d3 - d1;
        double d6 = d4 * d5;
        double d7 = d6 / (d2 + 1.0);
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use - array indexing */
        int val1 = array1[key_index];
        
        /* Second use - in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            v1 += array3[key_index];
            /* Force materialization */
            volatile int sink = key_index;
            (void)sink;
        }
        
        /* Third use - arithmetic */
        int val2 = array3[key_index] * 2;
        
        /* Fourth use - passed to function */
        use_int(key_index);
        
        /* Complex control flow with inner conditional */
        switch (outer % 5) {
            case 0:
                /* Use key_index again in different mode context */
                double d_val = array2[key_index];
                d_accumulator += d_val;
                /* Mixed mode usage */
                use_double(d_val);
                break;
            case 1:
                /* Different use pattern */
                val1 += key_index * 3;
                break;
            case 2:
                /* Create another recomputable expression */
                {
                    int another_index = (key_index * 3 + 7) % ARRAY_SIZE;
                    val2 += array1[another_index];
                    /* Force both values live */
                    volatile int sink2 = key_index + another_index;
                    (void)sink2;
                }
                break;
            case 3:
                /* Use in pointer arithmetic */
                use_ptr(&array1[key_index]);
                break;
            case 4:
                /* Nested loop to create cyclic data flow */
                {
                    int sum = 0;
                    for (int inner = 0; inner < 3; inner++) {
                        /* key_index used inside inner loop */
                        sum += array1[(key_index + inner) % ARRAY_SIZE];
                    }
                    val1 += sum;
                }
                break;
        }
        
        /* More register pressure variables */
        int v9 = v8 + key_index;
        int v10 = v9 * v7;
        double d8 = d7 + array2[key_index % (ARRAY_SIZE/2)];
        double d9 = d8 * d6;
        
        /* Use all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8 ^ v9 ^ v10;
        accumulator ^= val1 ^ val2;
        
        d_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9;
        
        /* Use volatile to force materialization */
        volatile int final_sink = key_index + outer;
        (void)final_sink;
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (accumulator), %f (d_accumulator)\n", 
           accumulator, d_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
