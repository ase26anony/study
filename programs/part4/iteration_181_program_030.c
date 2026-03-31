/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o early-remat-trigger
 * Alternative: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o early-remat-trigger
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
    volatile double daccumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < LOOP_ITERATIONS; outer++) {
        /* Create register pressure with many live variables */
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
        
        /* KEY COMPUTATION: Cheap to recompute, used multiple times */
        /* This is the candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        accumulator ^= val1;
        
        /* Second use: different array with offset */
        int val2 = array2[(key_index + 1) % ARRAY_SIZE];
        accumulator += val2;
        
        /* Third use: in conditional */
        if (key_index % 3 == 0) {
            v1 += array1[key_index % 100];
            use_int(v1);
        }
        
        /* Fourth use: double array indexing with mode mixing */
        double dval = darray1[key_index];
        daccumulator += dval;
        
        /* Fifth use: passed to function */
        use_int(key_index);
        
        /* Complex control flow that splits uses */
        switch (key_index % 5) {
            case 0:
                /* Use in case 0 */
                v2 += key_index;
                use_int(v2);
                break;
            case 1:
                /* Different use pattern */
                d1 += darray2[key_index];
                use_double(d1);
                break;
            case 2:
                /* Nested conditional with use */
                if (key_index > ARRAY_SIZE / 2) {
                    v3 = key_index * 2;
                    use_int(v3);
                } else {
                    v4 = key_index / 2;
                    use_int(v4);
                }
                break;
            case 3:
                /* Small inner loop creating cyclic flow */
                for (int inner = 0; inner < 3; inner++) {
                    /* Use key_index inside inner loop */
                    int temp = key_index + inner;
                    accumulator ^= temp;
                }
                break;
            case 4:
                /* goto creating non-trivial control flow */
                if (key_index % 2 == 0) {
                    goto label1;
                }
                v5 = key_index * 3;
                use_int(v5);
                label1:
                v6 = key_index + 100;
                use_int(v6);
                break;
        }
        
        /* Sixth use: after control flow merge */
        int val3 = array2[key_index];
        accumulator -= val3;
        
        /* Seventh use: in pointer calculation (different mode) */
        void* ptr = &array1[key_index];
        use_ptr(ptr);
        
        /* Eighth use: recomputed again for double operation */
        double dval2 = darray2[key_index];
        daccumulator *= (dval2 + 1.0);
        
        /* More register pressure variables */
        int v11 = v1 + v2;
        int v12 = v3 * v4;
        int v13 = v5 ^ v6;
        int v14 = v7 & v8;
        int v15 = v9 | v10;
        
        double d6 = d1 + d2;
        double d7 = d3 * d4;
        double d8 = d5 / 2.0;
        
        /* Force materialization */
        volatile int vsink1 = v11;
        volatile int vsink2 = v12;
        volatile int vsink3 = v13;
        volatile int vsink4 = v14;
        volatile int vsink5 = v15;
        
        volatile double dsink1 = d6;
        volatile double dsink2 = d7;
        volatile double dsink3 = d8;
        
        /* Use all variables to prevent dead code elimination */
        accumulator += v11 + v12 + v13 + v14 + v15;
        daccumulator += d6 + d7 + d8;
        
        /* Periodic function calls to create optimization barriers */
        if (outer % 1000 == 0) {
            use_int(accumulator);
            use_double(daccumulator);
        }
    }
    
    /* Final output to prevent entire loop elimination */
    printf("Result: %d (%.2f)\n", accumulator, daccumulator);
    
    return 0;
}
