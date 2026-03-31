/* early-remat-trigger.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy functions to prevent optimization */
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

int main(void) {
    const int ARRAY_SIZE = 1000;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory pressure */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = i * 2;
    }
    
    volatile int accumulator = 0;
    
    /* Main high-pressure loop */
    for (int outer = 0; outer < ITERATIONS; outer++) {
        /* Create many live variables to increase register pressure */
        int v1 = outer * 3;
        int v2 = outer + 17;
        int v3 = outer ^ 0x55AA;
        int v4 = outer % 13;
        int v5 = outer * 7;
        int v6 = outer - 42;
        int v7 = outer / 3;
        int v8 = outer << 2;
        int v9 = outer >> 1;
        int v10 = outer | 0xFF;
        
        double d1 = outer * 0.3;
        double d2 = outer + 0.7;
        double d3 = outer * 1.5;
        double d4 = outer / 2.0;
        double d5 = outer - 0.25;
        
        /* Key recomputable expression - cheap but used multiple times */
        /* This should be a candidate for rematerialization */
        int key_index = (outer * 7 + 123) % ARRAY_SIZE;
        
        /* Use key_index in multiple distinct operations */
        /* First use - array indexing */
        int val1 = array1[key_index];
        
        /* Force materialization with volatile */
        volatile int sink1 = key_index;
        
        /* Second use - in conditional */
        if (key_index > ARRAY_SIZE / 2) {
            /* Create complex control flow with inner block */
            int temp = array3[key_index];
            accumulator += temp;
            
            /* Third use - in arithmetic */
            int offset = key_index * 2;
            if (offset < ARRAY_SIZE) {
                /* Fourth use - different mode (pointer arithmetic) */
                use_ptr(&array2[offset]);
            }
            
            /* Mix in some double computations */
            double dval = array2[key_index];
            dval = use_double(dval);
            sink1 = (int)dval;
        } else {
            /* Alternative path that also uses key_index */
            /* Fifth use - in function call */
            int check = use_int(key_index);
            
            /* Sixth use - in another array index */
            double dval = array2[ARRAY_SIZE - 1 - key_index];
            accumulator += (int)dval;
            
            /* Create a small inner loop for cyclic data flow */
            int inner_sum = 0;
            for (int j = 0; j < 3; j++) {
                /* Seventh use - inside inner loop */
                inner_sum += key_index + j;
            }
            accumulator ^= inner_sum;
        }
        
        /* Eighth use - after conditional, in another computation */
        int final_index = (key_index + v1) % ARRAY_SIZE;
        accumulator += array1[final_index];
        
        /* Use all the variables to keep them live */
        accumulator += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        accumulator += (int)(d1 + d2 + d3 + d4 + d5);
        
        /* Another recomputable expression with different mode (double) */
        double recomputed_double = (outer * 0.7 + 45.3);
        
        /* Use double expression multiple times */
        double dresult1 = recomputed_double * 2.0;
        double dresult2 = recomputed_double / 3.0;
        
        /* Force double materialization */
        volatile double sink2 = recomputed_double;
        
        /* Use in conditional */
        if (recomputed_double > 100.0) {
            dresult1 = use_double(dresult1);
        } else {
            dresult2 = use_double(dresult2);
        }
        
        accumulator += (int)dresult1 + (int)dresult2;
        
        /* Create register pressure with switch statement */
        switch (outer % 5) {
            case 0:
                accumulator += key_index * 2;  /* Ninth use */
                break;
            case 1:
                accumulator += array3[key_index];  /* Tenth use */
                break;
            case 2:
                /* Use goto to create non-trivial control flow */
                if (key_index % 3 == 0) {
                    goto special_case;
                }
                accumulator += key_index;
                break;
            case 3:
                accumulator += val1;
                break;
            case 4:
                special_case:
                accumulator += key_index + 1;  /* Eleventh use */
                break;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
