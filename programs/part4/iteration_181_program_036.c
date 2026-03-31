/* early-remat-trigger.c
 * Designed to trigger uncovered lines in GCC's early-remat.cc
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024
#define LOOP_ITERATIONS 1000000

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

/* Complex control flow helper */
__attribute__((noinline)) int conditional_transform(int x, int y) {
    if (x > y) {
        return x * 2 - y;
    } else {
        return y * 3 + x;
    }
}

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
        darray2[i] = i * 1.5;
    }
    
    volatile int global_sink = 0;
    volatile double dglobal_sink = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer - 5;
        int v5 = outer / 2;
        int v6 = outer % 7;
        int v7 = outer * outer;
        int v8 = outer + 100;
        int v9 = outer - 50;
        int v10 = outer * 4;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = outer * 0.4;
        double d5 = outer * 0.5;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index multiple times in different ways */
        /* First use - array access */
        int val1 = array1[key_index];
        use_int(val1);
        
        /* Second use - in arithmetic */
        int transformed = key_index * 3 + 17;
        use_int(transformed);
        
        /* Third use - conditional with different mode (double) */
        double dval = darray1[key_index];
        use_double(dval);
        
        /* Complex control flow that splits uses of key_index */
        if (outer % 3 == 0) {
            /* Fourth use inside conditional block */
            int val2 = array2[key_index];
            use_int(val2);
            
            /* More register pressure variables */
            int v11 = v1 + v2;
            int v12 = v3 * v4;
            double d6 = d1 + d2;
            double d7 = d3 * d4;
            
            /* Use all variables to keep them live */
            use_int(v11);
            use_int(v12);
            use_double(d6);
            use_double(d7);
            
            /* Fifth use - passed to function */
            use_int(key_index);
            
            /* Inner conditional to create more CFG complexity */
            if (key_index % 5 == 0) {
                /* Sixth use - different computation */
                int val3 = key_index + array1[key_index % 100];
                use_int(val3);
                
                /* Small inner loop to create cyclic data flow */
                for (int inner = 0; inner < 3; inner++) {
                    /* Seventh use - in loop with different mode */
                    double dval2 = darray2[(key_index + inner) % ARRAY_SIZE];
                    dglobal_sink += dval2;
                }
            }
        } else if (outer % 3 == 1) {
            /* Alternative path with different key_index usage */
            /* Eighth use - pointer arithmetic */
            use_ptr(&array1[key_index]);
            
            /* More mixed-mode computations */
            int v13 = key_index * key_index;
            double d8 = key_index * 0.25;
            use_int(v13);
            use_double(d8);
        } else {
            /* Third path - switch statement for CFG complexity */
            switch (key_index % 4) {
                case 0:
                    /* Ninth use */
                    use_int(key_index + 1000);
                    break;
                case 1:
                    /* Tenth use */
                    use_double(key_index * 0.333);
                    break;
                case 2:
                    /* Eleventh use - in function call with computation */
                    use_int(conditional_transform(key_index, outer));
                    break;
                default:
                    /* Twelfth use */
                    global_sink += key_index;
                    break;
            }
        }
        
        /* Use all pressure variables before loop end */
        int sum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        double dsum = d1 + d2 + d3 + d4 + d5;
        
        /* Force materialization with volatile */
        volatile int vsink = sum;
        volatile double vdsink = dsum;
        
        /* Final use of key_index before loop iteration ends */
        /* Thirteenth use - ensures value is live across iteration */
        if (outer % 100 == 0) {
            global_sink += key_index;
        }
    }
    
    printf("Result: %d (%.2f)\n", global_sink, dglobal_sink);
    return 0;
}
