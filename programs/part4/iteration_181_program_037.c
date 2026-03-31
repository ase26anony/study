/* early-remat-trigger.c
 * Designed to trigger early rematerialization virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Opaque functions to prevent optimization */
#define NOINLINE __attribute__((noinline))

NOINLINE void use_int(int x) {
    volatile int sink = x;
    (void)sink;
}

NOINLINE void use_double(double x) {
    volatile double sink = x;
    (void)sink;
}

NOINLINE void use_ptr(void* p) {
    volatile void* sink = p;
    (void)sink;
}

/* Complex control flow helper */
NOINLINE int conditional_helper(int x, int y) {
    return (x > y) ? x - y : y - x;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Large arrays to create memory pressure */
    int* array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int* array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    double* array3 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = ARRAY_SIZE - i;
        array3[i] = i * 0.5;
    }
    
    volatile int accumulator = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live scalar variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = outer * 3;
        int v4 = outer / 2;
        int v5 = outer % 17;
        int v6 = v1 + v2;
        int v7 = v3 - v4;
        int v8 = v5 * 2;
        int v9 = v6 + v7;
        int v10 = v8 - v9;
        
        double d1 = outer * 0.1;
        double d2 = outer * 0.2;
        double d3 = outer * 0.3;
        double d4 = d1 + d2;
        double d5 = d3 - d1;
        double d6 = d4 * d5;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int temp = key_index;
        (void)temp;
        
        /* Complex control flow splitting uses of key_index */
        if (outer % 3 == 0) {
            /* Second use: different array indexing */
            int val2 = array2[key_index];
            
            /* Third use: in arithmetic */
            int val3 = val1 + val2 + key_index;
            
            /* Use in conditional */
            if (key_index % 2 == 0) {
                /* Fourth use: function call */
                use_int(key_index);
                
                /* Fifth use: more computation */
                int val4 = array1[key_index / 2] + key_index;
                accumulator ^= val4;
            } else {
                /* Alternative use path */
                int val5 = array2[key_index * 2 % ARRAY_SIZE] - key_index;
                accumulator ^= val5;
            }
            
            /* Small inner loop creating cyclic data flow */
            for (int inner = 0; inner < 3; inner++) {
                /* Sixth use: recomputed in inner loop */
                int inner_key = (outer * 7 + 123) % ARRAY_SIZE; /* Same computation! */
                int val6 = array1[inner_key] + inner;
                accumulator ^= val6;
                
                /* Use double variables to create different machine modes */
                double dval = array3[inner_key % (ARRAY_SIZE/2)];
                use_double(dval + d6);
            }
            
            /* Use goto to create additional control flow complexity */
            if (key_index % 5 == 0) {
                goto special_case;
            }
        } else if (outer % 3 == 1) {
            /* Different branch, still using key_index */
            double dval = array3[key_index % (ARRAY_SIZE/2)];
            use_double(dval + key_index);
            
            /* Seventh use: pointer arithmetic */
            void* ptr = &array1[key_index];
            use_ptr(ptr);
        } else {
            /* Third branch path */
            int val7 = key_index * key_index % 256;
            accumulator ^= val7;
        }
        
        /* Use all the scalar variables to keep them live */
        int sum_ints = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        double sum_doubles = d1 + d2 + d3 + d4 + d5 + d6;
        
        /* Force use to prevent optimization */
        volatile int vsink = sum_ints;
        volatile double dsink = sum_doubles;
        (void)vsink;
        (void)dsink;
        
        continue; /* Skip label for normal flow */
        
    special_case:
        /* Eighth use: in special case reached by goto */
        int val8 = array2[key_index] * 2;
        accumulator ^= val8;
        
        /* Use double computation with different mode */
        double special_d = array3[key_index] * 3.14;
        use_double(special_d);
    }
    
    /* Final result to prevent complete optimization */
    printf("Result: %d\n", accumulator);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
