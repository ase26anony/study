/* early-remat-trigger.c
 * Designed to trigger early rematerialization with virtual register creation
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

/* Dummy noinline functions to prevent optimization */
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

/* Complex control flow helper */
int __attribute__((noinline)) complex_cond(int a, int b) {
    volatile int v = a ^ b;
    return v & 1;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 100000;
    
    /* Arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int *array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i * 3;
        array2[i] = i * 1.5;
        array3[i] = &array1[i];
    }
    
    volatile int accumulator = 0;
    volatile double d_accumulator = 0.0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 2;
        int v2 = outer + 1;
        int v3 = v1 ^ v2;
        int v4 = v3 * 3;
        int v5 = v4 - v2;
        int v6 = v5 | v1;
        int v7 = v6 & 0xFF;
        int v8 = v7 << 2;
        int v9 = v8 >> 1;
        int v10 = v9 % 17;
        
        double d1 = outer * 0.1;
        double d2 = d1 + 1.5;
        double d3 = d2 * 2.0;
        double d4 = d3 / 1.7;
        double d5 = d4 - d1;
        double d6 = d5 * d2;
        double d7 = d6 + 3.14;
        double d8 = d7 - d4;
        double d9 = d8 / d3;
        double d10 = d9 * 0.5;
        
        /* KEY RECOMPUTABLE EXPRESSION - cheap to recompute, used multiple times */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use: array access */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use: in conditional with complex control flow */
        if (complex_cond(key_index, v3)) {
            /* Create inner control flow complexity */
            for (int inner = 0; inner < 3; inner++) {
                /* Use key_index again inside inner loop */
                double temp = array2[key_index] + inner;
                d_accumulator += temp;
                
                /* More register pressure variables */
                int t1 = v4 + inner;
                int t2 = t1 * key_index;
                int t3 = t2 ^ v7;
                volatile int sink2 = t3;
            }
            
            /* Third use: different mode (pointer) */
            void *ptr = array3[key_index];
            use_ptr(ptr);
            
            /* Fourth use: in arithmetic */
            int val2 = key_index * v5;
            accumulator ^= val2;
        } else {
            /* Alternative path that also uses key_index */
            switch (key_index % 4) {
                case 0:
                    /* Fifth use: as function argument */
                    use_int(key_index + v6);
                    break;
                case 1:
                    /* Sixth use: in array index with offset */
                    int idx = (key_index + 1) % ARRAY_SIZE;
                    val1 += array1[idx];
                    break;
                case 2:
                    /* Seventh use: in floating point context */
                    double d_val = key_index * 0.25;
                    d_accumulator += d_val;
                    break;
                default:
                    /* Eighth use: in pointer arithmetic */
                    int *p = &array1[key_index % (ARRAY_SIZE/2)];
                    *p = key_index;
                    break;
            }
        }
        
        /* Use double variables to create mixed mode pressure */
        d_accumulator += d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10;
        
        /* Use key_index one more time before loop ends */
        /* Ninth use: final computation */
        int final_val = key_index ^ v10;
        accumulator += final_val;
        
        /* Consume all integer variables to prevent optimization */
        volatile int all_vars = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        (void)all_vars;
        
        /* Mix in double usage */
        use_double(d10);
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d (accumulator), %f (double accumulator)\n", 
           accumulator, d_accumulator);
    
    return accumulator != 0 ? 0 : 1;
}
