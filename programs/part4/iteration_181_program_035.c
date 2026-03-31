/* early-remat-trigger.c
 * Designed to trigger GCC's early rematerialization pass to create
 * new virtual registers when register pressure is high.
 * Compile with: gcc -O2 -fno-omit-frame-pointer -fno-schedule-insns early-remat-trigger.c -o test
 * Or: gcc -O3 -fno-tree-vectorize -fno-unroll-loops early-remat-trigger.c -o test
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

/* Main function creating high register pressure */
int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 1000000;
    
    /* Arrays to create memory operations */
    int array1[ARRAY_SIZE];
    double array2[ARRAY_SIZE];
    int array3[ARRAY_SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array1[i] = i;
        array2[i] = i * 0.5;
        array3[i] = ARRAY_SIZE - i;
    }
    
    volatile int accumulator = 0;
    
    /* High register pressure loop */
    for (int i = 0; i < ITERATIONS; i++) {
        /* Create many live variables to increase register pressure */
        int v1 = i * 2;
        int v2 = i + 1;
        int v3 = i * 3;
        int v4 = i * 4;
        int v5 = i * 5;
        int v6 = i * 6;
        int v7 = i * 7;
        int v8 = i * 8;
        int v9 = i * 9;
        int v10 = i * 10;
        
        double d1 = i * 0.1;
        double d2 = i * 0.2;
        double d3 = i * 0.3;
        double d4 = i * 0.4;
        double d5 = i * 0.5;
        
        /* CHEAP RECOMPUTABLE EXPRESSION - candidate for rematerialization */
        /* This will be used multiple times in different contexts */
        int key_index = (i * 7 + 123) % ARRAY_SIZE;
        
        /* Force materialization of some variables */
        volatile int sink1 = v1;
        volatile double sink2 = d1;
        
        /* First use of key_index - array access */
        int val1 = array1[key_index];
        
        /* Complex control flow splitting uses of key_index */
        if (i % 3 == 0) {
            /* Second use of key_index - different array */
            double val2 = array2[key_index];
            use_double(val2);
            
            /* More computations to increase pressure */
            int v11 = v1 + v2;
            int v12 = v3 + v4;
            double d6 = d1 + d2;
            
            /* Third use of key_index - conditional computation */
            if (key_index % 2 == 0) {
                int val3 = array3[key_index];
                accumulator ^= val3;
            }
            
            use_int(v11);
            use_int(v12);
            use_double(d6);
            
            /* Inner loop to create cyclic data flow */
            for (int j = 0; j < 2; j++) {
                /* Fourth use of key_index - with different mode (pointer) */
                void* ptr = &array1[key_index];
                use_ptr(ptr);
                
                /* Mix computations */
                int temp = v5 + j;
                use_int(temp);
            }
        } else if (i % 3 == 1) {
            /* Alternative path still using key_index */
            int val4 = array3[key_index] * 2;
            accumulator += val4;
            
            /* More register pressure */
            double d7 = d3 * d4;
            int v13 = v6 * v7;
            use_double(d7);
            use_int(v13);
        } else {
            /* Third path - different computation with key_index */
            /* Fifth use of key_index - in arithmetic expression */
            int val5 = key_index * key_index;
            accumulator -= val5;
            
            /* Mixed mode computations */
            double d8 = key_index * 0.25;  /* Different mode: DF */
            use_double(d8);
        }
        
        /* Sixth use of key_index - after control flow reunites */
        /* Force recomputation by using in function call */
        use_int(key_index);
        
        /* More computations to keep variables live */
        int v14 = v8 + v9;
        double d9 = d4 + d5;
        volatile int sink3 = v14;
        volatile double sink4 = d9;
        
        /* Use all variables to prevent dead code elimination */
        accumulator ^= v1 ^ v2 ^ v3 ^ v4 ^ v5;
        accumulator += (int)(d1 + d2 + d3);
        
        /* Switch statement for additional control flow complexity */
        switch (i % 5) {
            case 0:
                /* Seventh use of key_index - in switch */
                array1[key_index % 100] = val1;
                break;
            case 1:
                use_int(v6);
                break;
            case 2:
                use_int(v7);
                break;
            case 3:
                use_int(v8);
                break;
            case 4:
                /* Eighth use of key_index - final use */
                int final_val = key_index + v10;
                use_int(final_val);
                break;
        }
    }
    
    printf("Result: %d\n", accumulator);
    return 0;
}
