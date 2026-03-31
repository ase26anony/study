/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Use volatile to prevent optimization */
static volatile int global_counter = 0;

/* Function with memory aliasing - no restrict keyword */
static inline void compute_loop(float *a, float *b, double *c, int *d, int n) {
    int i;
    float temp_f;
    double temp_d;
    int temp_i;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operation with carried dependency */
        temp_i = d[i] + global_counter;
        
        /* Floating-point operations */
        temp_f = a[i] * 1.2345f + b[i];
        
        /* Double precision operation */
        temp_d = c[i] / 3.14159 + temp_f;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different operation path */
            temp_f = temp_f * 2.0f - 1.0f;
            temp_d = temp_d + 1.0;
            temp_i = temp_i | 0x0F;
        } else if (i % 13 == 0) {
            /* Another branch */
            temp_f = temp_f / 1.5f;
            temp_d = temp_d * 0.9;
            temp_i = temp_i & 0xF0;
        } else {
            /* Default path */
            temp_f = temp_f + 0.5f;
            temp_d = temp_d - 0.1;
            temp_i = temp_i ^ 0x55;
        }
        
        /* Memory operations with potential aliasing */
        a[i] = temp_f;
        b[i] = temp_f * 0.8f;
        c[i] = temp_d;
        d[i] = temp_i;
        
        /* Volatile update to prevent dead code elimination */
        global_counter += (i & 0x1);
    }
}

/* Another computation function to increase scheduling complexity */
static inline void mixed_operations(int *restrict x, float *restrict y, 
                                   double *z, int m) {
    int j;
    double acc_d = 0.0;
    float acc_f = 0.0f;
    int acc_i = 0;
    
    for (j = 0; j < m; j++) {
        /* Mixed precision calculations */
        acc_f = y[j] * 2.71828f;
        acc_d = z[j] * 3.14159;
        acc_i = x[j] * 17;
        
        /* Complex expression with multiple operations */
        y[j] = acc_f + (float)acc_d;
        z[j] = acc_d - (double)acc_f;
        x[j] = acc_i + (int)(acc_f * 100.0f);
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
        
        /* Another conditional */
        if (j % 11 == 0) {
            acc_f = acc_f * 0.5f;
            acc_d = acc_d / 2.0;
        }
    }
}

int main(void) {
    float array_a[SIZE];
    float array_b[SIZE];
    double array_c[SIZE];
    int array_d[SIZE];
    int i, result = 0;
    
    /* Initialize arrays with pseudo-random values */
    srand(time(NULL));
    for (i = 0; i < SIZE; i++) {
        array_a[i] = (float)(rand() % 1000) / 10.0f;
        array_b[i] = (float)(rand() % 1000) / 20.0f;
        array_c[i] = (double)(rand() % 1000) / 30.0;
        array_d[i] = rand() % 256;
    }
    
    /* Perform multiple iterations to create hot loop */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call the hot loop function */
        compute_loop(array_a, array_b, array_c, array_d, SIZE);
        
        /* Periodically call mixed operations */
        if (i % 100 == 0) {
            mixed_operations(array_d, array_a, array_c, SIZE);
        }
        
        /* Modify global state */
        global_counter = (global_counter + 1) & 0xFF;
    }
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        result ^= array_d[i];
        result += (int)(array_a[i] * 100);
        result ^= (int)(array_c[i] * 10);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return 0;
}
