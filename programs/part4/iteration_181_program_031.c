/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers in high register pressure situations.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or for more pressure: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
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

__attribute__((noinline)) void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 1000000

int main(void) {
    /* Large arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    int array2[ARRAY_SIZE];
    double darray1[ARRAY_SIZE];
    double darray2[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        darray1[i] = i * 0.5;
        darray2[i] = (ARRAY_SIZE - i) * 0.25;
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer - 5;
        int v5 = outer + 10;
        int v6 = outer * 7;
        int v7 = outer / 2;
        int v8 = outer % 13;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        
        /* Second use: different array with offset */
        int val2 = array2[(key_index + 5) % ARRAY_SIZE];
        
        /* Third use: in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            /* Create inner control flow complexity */
            int v9 = v1 + v2;
            int v10 = v3 * v4;
            
            /* Use key_index again inside branch */
            double dval1 = darray1[key_index];
            d_accumulator += dval1;
            
            /* More computations to increase pressure */
            for (int inner = 0; inner < 3; inner++) {
                /* Small inner loop creates cyclic data flow */
                v9 += inner;
                v10 -= key_index;  /* Use key_index in inner loop */
            }
            
            use_int(v9);
            use_int(v10);
        } else {
            /* Alternative path also uses key_index */
            double dval2 = darray2[key_index];
            d_accumulator -= dval2;
            
            /* Switch statement for additional control flow complexity */
            switch (key_index % 4) {
                case 0:
                    v5 += key_index;
                    break;
                case 1:
                    v6 -= key_index;
                    break;
                case 2:
                    v7 *= (key_index + 1);
                    break;
                case 3:
                    v8 = key_index / 2;
                    break;
            }
        }
        
        /* Fourth use: after conditional, in function call */
        use_int(key_index);
        
        /* Fifth use: pointer arithmetic */
        int* ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Sixth use: mixed-mode computation */
        double d_key = (double)key_index;
        double d_result = d_key * d1 + d2;
        use_double(d_result);
        
        /* Consume all variables to prevent optimization */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5 ^ v6 ^ v7 ^ v8;
        accumulator += val1 + val2;
        
        /* More double computations to create DF mode registers */
        double d6 = d1 * d2 + d3;
        double d7 = d4 / (d5 + 1.0);
        double d8 = d6 - d7 + d_key;  /* Use d_key again */
        
        d_accumulator += d6 + d7 + d8;
        
        /* Additional conditional with goto to create complex CFG */
        if (outer % 100 == 0) {
            /* Label creates a potential jump target */
            special_case:
            v1 += 1000;
            v2 -= 500;
            /* Use key_index in goto block */
            accumulator += key_index * 2;
        }
        
        /* Another use of key_index before loop ends */
        if (key_index % 3 == 0) {
            goto special_case;
        }
    }
    
    /* Prevent entire computation from being optimized away */
    printf("Result: %d (accumulator), %f (double accumulator)\n", 
           accumulator, d_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
