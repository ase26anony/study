/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
double high_pressure_loop(double *arr, int size) {
    double sum = 0.0;
    double t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
    
    /* Create many independent computations to give scheduler choices */
    for (int i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = arr[i] * 1.1;
        t2 = arr[i] * 1.2;
        t3 = arr[i] * 1.3;
        t4 = arr[i] * 1.4;
        t5 = arr[i] * 1.5;
        
        /* Group 2: More independent operations */
        a1 = t1 + t2;
        a2 = t2 + t3;
        a3 = t3 + t4;
        a4 = t4 + t5;
        a5 = t5 + t1;
        
        /* Group 3: Cross dependencies to create priority differences */
        t6 = a1 / (a2 + 0.001);  /* Division creates longer latency */
        t7 = a2 / (a3 + 0.001);
        t8 = a3 / (a4 + 0.001);
        t9 = a4 / (a5 + 0.001);
        t10 = a5 / (a1 + 0.001);
        
        /* Mix with volatile accesses to create delays */
        a6 = t6 * vol_f1;
        a7 = t7 * vol_f2;
        a8 = t8 * vol_f3;
        a9 = t9 * vol_f1;
        a10 = t10 * vol_f2;
        
        /* Final accumulation with artificial dependencies */
        sum += a6 + a7 + a8 + a9 + a10;
        
        /* Inline assembly to clobber registers and increase pressure */
        __asm__ volatile (
            "movq $0, %%rax\n\t"
            "movq $0, %%rbx\n\t"
            "movq $0, %%rcx\n\t"
            "movq $0, %%rdx\n\t"
            "movq $0, %%rsi\n\t"
            "movq $0, %%rdi\n\t"
            "movq $0, %%r8\n\t"
            "movq $0, %%r9\n\t"
            "movq $0, %%r10\n\t"
            "movq $0, %%r11\n\t"
            "movq $0, %%r12\n\t"
            "movq $0, %%r13\n\t"
            "movq $0, %%r14\n\t"
            "movq $0, %%r15\n\t"
            : /* no outputs */
            : /* no inputs */
            : "rax", "rbx", "rcx", "rdx", "rsi", "rdi",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    return sum;
}

/* Function with mixed dependencies and resource conflicts */
void mixed_dependency(int *arr_int, float *arr_float, int size) {
    int r1, r2, r3, r4, r5, r6, r7, r8, r9, r10;
    float f1, f2, f3, f4, f5, f6, f7, f8, f9, f10;
    
    /* Create independent instruction groups for scheduler candidates */
    for (int i = 0; i < size; i++) {
        /* Integer operations group - independent from each other */
        r1 = arr_int[i] + vol_a;
        r2 = arr_int[(i + 1) % size] - vol_b;
        r3 = arr_int[(i + 2) % size] * vol_c;
        r4 = arr_int[(i + 3) % size] / (vol_d + 1);  /* Division for delay */
        r5 = arr_int[(i + 4) % size] & 0xFF;
        
        /* Floating point operations - compete for FP units */
        f1 = arr_float[i] * 1.2345f;
        f2 = arr_float[(i + 1) % size] / 2.3456f;  /* Division creates delay */
        f3 = arr_float[(i + 2) % size] + 3.4567f;
        f4 = arr_float[(i + 3) % size] - 4.5678f;
        f5 = arr_float[(i + 4) % size] * 5.6789f;
        
        /* Cross-type dependencies to create scheduling constraints */
        r6 = (int)(f1 * 100.0f) + r1;
        r7 = (int)(f2 * 100.0f) + r2;
        r8 = (int)(f3 * 100.0f) + r3;
        r9 = (int)(f4 * 100.0f) + r4;
        r10 = (int)(f5 * 100.0f) + r5;
        
        /* Memory operations with potential aliasing */
        arr_int[i] = r6 + r7;
        arr_float[i] = (float)(r8 + r9 + r10) * 0.01f;
        
        /* Conditional to create control flow and priority differences */
        if (arr_int[i] > 1000) {
            f6 = sqrtf(arr_float[i]);  /* Long latency sqrt */
            f7 = sinf(arr_float[i]);   /* Long latency sin */
            arr_float[i] = f6 + f7;
        } else {
            f8 = arr_float[i] * arr_float[i];
            f9 = f8 * 2.0f;
            arr_float[i] = f9;
        }
        
        /* More independent operations to fill candidate array */
        f10 = arr_float[(i + 5) % size] * 0.5f;
        arr_float[(i + 5) % size] = f10 + (float)vol_a;
    }
}

/* Function with unrolled loops for many independent instructions */
void unrolled_computations(double *arr1, double *arr2, double *arr3, int size) {
    /* Manual unrolling creates many independent instructions */
    for (int i = 0; i < size; i += 8) {
        /* Eight independent computation groups */
        arr3[i] = arr1[i] * arr2[i] + vol_f1;
        arr3[i+1] = arr1[i+1] * arr2[i+1] + vol_f2;
        arr3[i+2] = arr1[i+2] * arr2[i+2] + vol_f3;
        arr3[i+3] = arr1[i+3] * arr2[i+3] + vol_f1;
        arr3[i+4] = arr1[i+4] * arr2[i+4] + vol_f2;
        arr3[i+5] = arr1[i+5] * arr2[i+5] + vol_f3;
        arr3[i+6] = arr1[i+6] * arr2[i+6] + vol_f1;
        arr3[i+7] = arr1[i+7] * arr2[i+7] + vol_f2;
        
        /* More operations on same data to create register pressure */
        arr1[i] = arr3[i] * 0.9;
        arr1[i+1] = arr3[i+1] * 0.8;
        arr1[i+2] = arr3[i+2] * 0.7;
        arr1[i+3] = arr3[i+3] * 0.6;
        arr1[i+4] = arr3[i+4] * 0.5;
        arr1[i+5] = arr3[i+5] * 0.4;
        arr1[i+6] = arr3[i+6] * 0.3;
        arr1[i+7] = arr3[i+7] * 0.2;
    }
}

int main() {
    double *arr1, *arr2, *arr3;
    int *arr_int;
    float *arr_float;
    double total_sum = 0.0;
    
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    arr1 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    arr2 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    arr3 = (double*)malloc(ARRAY_SIZE * sizeof(double));
    arr_int = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr_float = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = (double)rand() / RAND_MAX * 100.0;
        arr2[i] = (double)rand() / RAND_MAX * 100.0;
        arr_int[i] = rand() % 1000;
        arr_float[i] = (float)rand() / RAND_MAX * 100.0f;
    }
    
    /* Perform multiple iterations to ensure hot code scheduling */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions that create different scheduling scenarios */
        total_sum += high_pressure_loop(arr1, ARRAY_SIZE / 4);
        
        mixed_dependency(arr_int, arr_float, ARRAY_SIZE / 8);
        
        unrolled_computations(arr1, arr2, arr3, ARRAY_SIZE / 16);
        
        /* Update volatile variables to create new dependencies */
        vol_a = (vol_a * 3 + 1) % 100;
        vol_b = (vol_b * 5 + 2) % 100;
        vol_c = (vol_c * 7 + 3) % 100;
        vol_d = (vol_d * 11 + 4) % 100;
        vol_f1 = (float)vol_a * 0.1f;
        vol_f2 = (float)vol_b * 0.2f;
        vol_f3 = (float)vol_c * 0.3f;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %f\n", total_sum);
    printf("Final values: arr_int[0]=%d, arr_float[0]=%f\n", 
           arr_int[0], arr_float[0]);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr_int);
    free(arr_float);
    
    return 0;
}
