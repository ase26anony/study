/* File 1: Floating-point intensive computations with nested loops */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 100

/* Volatile to prevent optimization */
volatile int g_seed = 42;

/* Complex floating-point computation with data dependencies */
void compute_fp_matrix(float* restrict a, float* restrict b, float* restrict c, int n) {
    int i, j, k;
    
    /* Triple nested loop creates scheduling pressure */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            float sum = 0.0f;
            for (k = 0; k < n; k++) {
                /* Mixed operations: multiply, add, trigonometric */
                sum += a[i * n + k] * b[k * n + j] + 
                       sinf(a[i * n + k] * 0.01f) * cosf(b[k * n + j] * 0.01f);
            }
            c[i * n + j] = sum * expf(sum * 0.001f);
        }
    }
}

/* Function with control flow and memory dependencies */
void process_signal(float* signal, int length) {
    float prev = 0.0f;
    float prev2 = 0.0f;
    int i;
    
    /* IIR-like filter with data dependencies */
    for (i = 0; i < length; i++) {
        float current = signal[i];
        
        /* Branch creates scheduling barriers */
        if (i > 1) {
            signal[i] = current * 0.5f + prev * 0.3f + prev2 * 0.2f;
            
            /* Additional conditional computation */
            if (signal[i] > 1.0f) {
                signal[i] = 1.0f / signal[i];
            } else if (signal[i] < -1.0f) {
                signal[i] = -1.0f - signal[i];
            }
        } else if (i == 1) {
            signal[i] = current * 0.7f + prev * 0.3f;
        }
        
        prev2 = prev;
        prev = current;
        
        /* Small inner loop for additional scheduling complexity */
        int j;
        for (j = 0; j < 4; j++) {
            signal[i] += sinf(signal[i] * j * 0.1f) * 0.01f;
        }
    }
}

/* Main computation function */
void test_fp_computation(int size) {
    float* a = malloc(size * size * sizeof(float));
    float* b = malloc(size * size * sizeof(float));
    float* c = malloc(size * size * sizeof(float));
    float* signal = malloc(size * sizeof(float));
    
    if (!a || !b || !c || !signal) return;
    
    /* Initialize with pseudo-random values */
    int i;
    for (i = 0; i < size * size; i++) {
        a[i] = (float)((i * 17 + g_seed) % 100) * 0.01f;
        b[i] = (float)((i * 23 + g_seed) % 100) * 0.01f;
    }
    
    for (i = 0; i < size; i++) {
        signal[i] = sinf(i * 0.1f);
    }
    
    /* Multiple iterations to increase scheduling opportunities */
    for (i = 0; i < ITERATIONS / 10; i++) {
        compute_fp_matrix(a, b, c, size / 4);
        process_signal(signal, size);
        
        /* Feedback loop - use results in next iteration */
        int j;
        for (j = 0; j < size * size; j++) {
            a[j] = a[j] * 0.9f + c[j] * 0.1f;
        }
    }
    
    free(a);
    free(b);
    free(c);
    free(signal);
}
