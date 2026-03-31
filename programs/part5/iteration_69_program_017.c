/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr1, double *arr2, double *arr3, int n) {
    double sum = 0.0;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    
    /* Unrolled loop with many independent calculations to create scheduling candidates */
    for (int i = 0; i < n - 10; i += 10) {
        /* Group 1: Independent floating point operations */
        t1 = arr1[i] * arr2[i] + arr3[i];
        t2 = arr1[i+1] / (arr2[i+1] + 1.0);  /* FP divide for latency */
        t3 = arr1[i+2] - arr2[i+2] * arr3[i+2];
        t4 = sqrt(arr1[i+3] * arr1[i+3] + arr2[i+3] * arr2[i+3]); /* sqrt for latency */
        
        /* Group 2: More independent operations */
        t5 = arr1[i+4] * 3.14159;
        t6 = arr2[i+5] * 2.71828;
        t7 = arr3[i+6] * 1.41421;
        t8 = t1 + t2 - t3;
        
        /* Group 3: Creating dependencies across groups */
        t9 = t4 * t5 + t6;
        t10 = t7 * t8 - t9;
        
        /* Register pressure: keep many values live */
        r1 = t1 + vol_f1;  /* volatile creates memory dependency */
        r2 = t2 * vol_f2;
        r3 = t3 - vol_f3;
        r4 = t4 / (vol_f1 + 1.0f);
        r5 = t5 * r1;
        r6 = t6 + r2;
        r7 = t7 - r3;
        r8 = t8 * r4;
        r9 = t9 + r5;
        r10 = t10 * r6;
        
        /* Use all results to prevent elimination */
        sum += r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
        
        /* Memory operations with potential aliasing */
        arr1[i] = r1;
        arr2[i+1] = r2;
        arr3[i+2] = r3;
    }
    
    return sum;
}

/* Function with mixed dependencies and resource conflicts */
int mixed_dependency(int *ints, float *floats, short *shorts, int n) {
    int result = 0;
    
    /* Create artificial resource conflicts */
    for (int i = 0; i < n; i++) {
        /* Integer operations */
        int a = ints[i] + vol_a;
        int b = ints[i] * vol_b;
        int c = a ^ b;  /* XOR creates different dependency */
        int d = c << (i & 3);  /* Variable shift */
        
        /* Floating point operations - compete for FP units */
        float f1 = floats[i] * 1.5f;
        float f2 = floats[i] / 3.0f;  /* FP divide for latency */
        float f3 = f1 + f2;
        float f4 = f3 * vol_f1;
        
        /* Mixed type operations causing delays */
        result += (int)(f4) + d;
        
        /* Volatile accesses create memory barriers */
        ints[i] = result & vol_c;
        floats[i] = f4 * vol_f2;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl %0, %%eax\n\t"
            "addl %1, %%eax\n\t"
            "movl %%eax, %0\n\t"
            : "+r" (result)
            : "r" (shorts[i])
            : "%eax", "cc"
        );
    }
    
    return result;
}

/* Function with control flow to create priority differences */
void control_flow_pattern(int *data, int n) {
    int i = 0;
    while (i < n) {
        /* Different paths create different instruction priorities */
        if (data[i] > 1000) {
            /* Critical path with many operations */
            int t1 = data[i] * 3;
            int t2 = t1 / 2;  /* Integer divide for latency */
            int t3 = t2 + vol_d;
            int t4 = t3 << 2;
            data[i] = t4 - 5;
            i += 2;  /* Skip ahead */
        } else if (data[i] > 500) {
            /* Medium priority path */
            data[i] = (data[i] * 2) | 1;
            i++;
        } else {
            /* Low priority path - simple operation */
            data[i] = data[i] + vol_a;
            i++;
        }
        
        /* Loop with break creates scheduling boundaries */
        if (i > n/2 && (data[i-1] & 1)) {
            break;
        }
    }
}

/* Main function that creates the hot loops for scheduler analysis */
int main() {
    /* Allocate and initialize data */
    double *arr1 = malloc(ARRAY_SIZE * sizeof(double));
    double *arr2 = malloc(ARRAY_SIZE * sizeof(double));
    double *arr3 = malloc(ARRAY_SIZE * sizeof(double));
    int *int_data = malloc(ARRAY_SIZE * sizeof(int));
    float *float_data = malloc(ARRAY_SIZE * sizeof(float));
    short *short_data = malloc(ARRAY_SIZE * sizeof(short));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (double)rand() / RAND_MAX * 100.0;
        arr2[i] = (double)rand() / RAND_MAX * 100.0;
        arr3[i] = (double)rand() / RAND_MAX * 100.0;
        int_data[i] = rand() % 2000;
        float_data[i] = (float)rand() / RAND_MAX * 500.0f;
        short_data[i] = rand() % 1000;
    }
    
    double total_sum = 0.0;
    int int_result = 0;
    
    /* Hot loop that will be analyzed by the scheduler */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        total_sum += high_pressure_loop(arr1, arr2, arr3, ARRAY_SIZE);
        int_result += mixed_dependency(int_data, float_data, short_data, ARRAY_SIZE/10);
        control_flow_pattern(int_data, ARRAY_SIZE/5);
        
        /* Modify volatile variables to change dependencies */
        if (iter % 100 == 0) {
            vol_a = (vol_a * 3) % 100;
            vol_f1 = sin(vol_f1 * 0.1f);
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: sum=%f, int_result=%d\n", total_sum, int_result);
    printf("Sample values: arr1[0]=%f, int_data[0]=%d\n", arr1[0], int_data[0]);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(int_data);
    free(float_data);
    free(short_data);
    
    return 0;
}
