/* sel-sched-test.c
 * Program designed to trigger selective scheduler debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -dS -fdump-rtl-all sel-sched-test.c -o sel-sched-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization */
volatile int g_volatile_counter = 0;
volatile double g_volatile_double = 1.0;

/* Arrays with potential aliasing */
static double data_a[SIZE];
static double data_b[SIZE];
static int data_c[SIZE];
static float data_d[SIZE];

/* Function with memory aliasing - restrict and non-restrict pointers */
static inline double process_element(double *restrict a, double *b, 
                                     int *c, float *d, int idx) {
    double result = 0.0;
    
    /* Complex arithmetic with mixed types */
    double temp1 = a[idx] * 1.234567;
    float temp2 = d[idx] / 3.14159f;
    int temp3 = c[idx] + idx * 7;
    
    /* Conditional creating multiple basic blocks */
    if (idx % 7 == 0) {
        result = temp1 + temp2 * 2.5;
        c[idx] = temp3 ^ 0x55AA55AA;  /* Memory store */
    } else if (idx % 13 == 0) {
        result = temp1 - temp2 / 1.618;
        c[idx] = temp3 & 0x00FF00FF;
    } else {
        result = temp1 * temp2 + temp3;
        c[idx] = temp3 | 0x12345678;
    }
    
    /* Volatile operation to prevent dead code elimination */
    g_volatile_counter += (int)result;
    
    return result;
}

/* Hot loop with carried dependencies and mixed operations */
static inline void compute_loop(double *restrict out, 
                                double *in1, double *in2,
                                int *counter, float *float_data,
                                int start, int end) {
    double accumulator = g_volatile_double;
    int int_accumulator = 0;
    
    /* Loop with carried dependency through accumulator */
    for (int i = start; i < end; i++) {
        /* Multiple independent operations */
        double val1 = in1[i] * 2.71828;
        double val2 = in2[i] / 1.41421;
        float val3 = float_data[i] + i * 0.01f;
        
        /* Memory load with potential aliasing */
        int counter_val = counter[i];
        
        /* Complex arithmetic chain */
        double intermediate = val1 + val2;
        intermediate = intermediate * (1.0 + val3);
        
        /* Integer operations mixed with FP */
        int_accumulator += counter_val ^ i;
        int_accumulator *= 1103515245;
        int_accumulator += 12345;
        
        /* Conditional store creating control flow */
        if (int_accumulator % 11 == 0) {
            out[i] = intermediate * 0.99;
        } else {
            out[i] = intermediate * 1.01;
        }
        
        /* Process element with function call */
        double processed = process_element(in1, in2, counter, float_data, i);
        
        /* Update accumulator with carried dependency */
        accumulator = accumulator * 0.999 + processed * 0.001;
        
        /* More mixed operations */
        float_data[i] = (float)(accumulator * 0.5);
        counter[i] = int_accumulator & 0x7FFFFFFF;
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : : "memory");
    }
    
    /* Store final accumulator to volatile */
    g_volatile_double = accumulator;
}

/* Initialize data with pseudo-random values */
static void initialize_data(void) {
    for (int i = 0; i < SIZE; i++) {
        data_a[i] = (i * 1.2345) / (i + 1);
        data_b[i] = (i * 0.9876) / (i + 2);
        data_c[i] = i * 1103515245 + 12345;
        data_d[i] = (float)(i * 0.4567);
    }
}

int main(void) {
    double output[SIZE];
    uint64_t checksum = 0;
    
    /* Initialize data */
    initialize_data();
    
    /* Perform multiple iterations to create hot loop */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call the hot loop function multiple times with different ranges */
        compute_loop(output, data_a, data_b, data_c, data_d, 
                    0, SIZE / 2);
        compute_loop(output + SIZE / 2, data_a + SIZE / 2, 
                    data_b + SIZE / 2, data_c + SIZE / 2, 
                    data_d + SIZE / 2, SIZE / 2, SIZE);
        
        /* Update data to create varying patterns */
        for (int i = 0; i < SIZE; i++) {
            data_a[i] = data_a[i] * 0.99 + output[i] * 0.01;
            data_b[i] = data_b[i] * 1.01 - output[i] * 0.005;
            
            /* Simple checksum to prevent optimization */
            checksum ^= *(uint64_t*)&output[i];
            checksum += data_c[i];
        }
        
        /* Periodic volatile update */
        if (iter % 1000 == 0) {
            g_volatile_counter = iter;
        }
    }
    
    /* Final computation with different parameters */
    for (int i = 0; i < SIZE; i += 8) {
        /* Unrolled loop with mixed operations */
        double sum = 0.0;
        for (int j = 0; j < 8 && (i + j) < SIZE; j++) {
            sum += data_a[i + j] * data_b[i + j];
            sum -= data_d[i + j];
            sum *= 1.0001;
        }
        output[i] = sum;
        
        /* More complex integer operations */
        int complex_int = data_c[i];
        complex_int = (complex_int << 3) | (complex_int >> 29);
        complex_int ^= 0xDEADBEEF;
        data_c[i] = complex_int;
        
        checksum ^= complex_int;
    }
    
    /* Print checksum to ensure computation isn't optimized away */
    printf("Final checksum: 0x%016llX\n", (unsigned long long)checksum);
    printf("Volatile counter: %d\n", g_volatile_counter);
    printf("Volatile double: %f\n", g_volatile_double);
    
    return 0;
}
