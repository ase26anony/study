/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr, int size) {
    double sum = 0.0;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double u1, u2, u3, u4, u5, u6, u7, u8, u9, u10;
    double v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    
    /* Many independent calculations to create scheduling candidates */
    for (int i = 0; i < size; i++) {
        /* Group 1: Independent FP operations */
        t1 = arr[i] * 1.1;
        t2 = arr[i] * 1.2;
        t3 = arr[i] * 1.3;
        t4 = arr[i] * 1.4;
        t5 = arr[i] * 1.5;
        
        /* Group 2: More independent operations */
        u1 = t1 + t2;
        u2 = t3 + t4;
        u3 = t5 * vol_f1;  /* Volatile dependency creates delay possibility */
        u4 = u1 * u2;
        u5 = u3 / (vol_f2 + 0.001f);  /* FP divide - long latency */
        
        /* Group 3: Mixed operations */
        v1 = sin(u1);      /* Function call - creates scheduling boundary */
        v2 = cos(u2);
        v3 = u3 * u4 * u5;
        v4 = v1 + v2 + v3;
        v5 = sqrt(fabs(v4));
        
        /* Use all results to keep them live */
        sum += v1 + v2 + v3 + v4 + v5 + t1 + t2 + t3 + t4 + t5;
        
        /* Inline asm to clobber registers and force spills */
        __asm__ volatile (
            "movq $0, %%rax\n"
            "movq $0, %%rbx\n"
            "movq $0, %%rcx\n"
            "movq $0, %%rdx\n"
            "movq $0, %%rsi\n"
            "movq $0, %%rdi\n"
            : : : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "memory"
        );
    }
    
    return sum;
}

/* Function with artificial dependencies and resource conflicts */
void mixed_dependency(int *arr_int, float *arr_float, int size) {
    int a = vol_a, b = vol_b, c = vol_c, d = vol_d;
    float f1 = vol_f1, f2 = vol_f2, f3 = vol_f3;
    
    /* Create chains of dependencies with different latencies */
    for (int i = 0; i < size; i++) {
        /* Integer chain - creates priority differences */
        int x1 = arr_int[i] + a;
        int x2 = x1 * b;
        int x3 = x2 / (c + 1);     /* Integer divide - variable latency */
        int x4 = x3 ^ d;
        int x5 = x4 << 2;
        
        /* Floating-point chain - competes for different units */
        float y1 = arr_float[i] * f1;
        float y2 = y1 / f2;        /* FP divide - long latency */
        float y3 = y2 + f3;
        float y4 = y3 * y1;
        float y5 = y4 - y2;
        
        /* Memory operations with aliasing */
        arr_int[i] = x5 + (int)y5;
        arr_float[i] = y5 * 0.5f;
        
        /* Volatile memory access - creates scheduling barrier */
        int tmp = vol_a;
        vol_b = tmp + i;
        
        /* Conditional to create control flow priority differences */
        if (x5 > 1000) {
            vol_c = x5;
            y5 = y5 * 2.0f;
        } else {
            vol_d = x5;
            y5 = y5 * 0.5f;
        }
        
        /* More independent operations for candidate selection */
        float z1 = y5 * 1.1f;
        float z2 = y5 * 1.2f;
        float z3 = y5 * 1.3f;
        float z4 = y5 * 1.4f;
        
        /* Use results */
        arr_float[i] += z1 + z2 + z3 + z4;
    }
}

/* Function with many independent instructions for candidate array */
void independent_instructions(double *arr, int size) {
    /* Large basic block with no dependencies between groups */
    for (int i = 0; i < size; i += 8) {
        /* Group A - completely independent */
        double a1 = arr[i] * 1.01;
        double a2 = arr[i+1] * 1.02;
        double a3 = arr[i+2] * 1.03;
        double a4 = arr[i+3] * 1.04;
        
        /* Group B - independent from A */
        double b1 = arr[i+4] * 2.01;
        double b2 = arr[i+5] * 2.02;
        double b3 = arr[i+6] * 2.03;
        double b4 = arr[i+7] * 2.04;
        
        /* Group C - mixes results but still has parallelism */
        double c1 = a1 + b1;
        double c2 = a2 + b2;
        double c3 = a3 + b3;
        double c4 = a4 + b4;
        
        /* Group D - more mixing */
        double d1 = c1 * c2;
        double d2 = c3 * c4;
        double d3 = d1 / (c1 + 1.0);
        double d4 = d2 / (c4 + 1.0);
        
        /* Store results - creates memory dependencies */
        arr[i] = d1;
        arr[i+1] = d2;
        arr[i+2] = d3;
        arr[i+3] = d4;
        arr[i+4] = c1;
        arr[i+5] = c2;
        arr[i+6] = c3;
        arr[i+7] = c4;
    }
}

int main() {
    double *arr_double = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *arr_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    /* Initialize with random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr_double[i] = (double)rand() / RAND_MAX * 100.0;
        arr_int[i] = rand() % 1000;
        arr_float[i] = (float)rand() / RAND_MAX * 50.0f;
    }
    
    double total_sum = 0.0;
    
    /* Performance-critical loop that will be scheduled */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different scheduling characteristics */
        total_sum += high_pressure_loop(arr_double, ARRAY_SIZE / 4);
        mixed_dependency(arr_int, arr_float, ARRAY_SIZE / 8);
        independent_instructions(arr_double, ARRAY_SIZE / 2);
        
        /* Modify volatile variables to change dependencies */
        vol_a = (vol_a * 13 + 17) % 23;
        vol_b = (vol_b * 11 + 19) % 29;
        vol_f1 = vol_f1 * 1.1f;
        vol_f2 = vol_f2 * 0.9f;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %f\n", total_sum);
    printf("Final volatile values: %d %d %f %f\n", 
           vol_a, vol_b, vol_f1, vol_f2);
    
    free(arr_double);
    free(arr_int);
    free(arr_float);
    
    return 0;
}
