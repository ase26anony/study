/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline double compute_loop(double *restrict arr1, double *arr2, 
                                  float *restrict farr, int *iarr, 
                                  int start, int end, int modifier) {
    double sum = 0.0;
    float fsum = 0.0f;
    int isum = 0;
    
    /* Hot loop with mixed operations and dependencies */
    for (int i = start; i < end; i++) {
        /* Create carried dependency */
        sum = sum * 0.99 + arr1[i];
        
        /* Independent floating-point operations */
        double temp = arr2[i] * 1.5;
        fsum += (float)temp;
        
        /* Integer operations with conditional */
        isum += iarr[i] & 0xFF;
        if (i % 7 == 0) {
            /* Different execution path */
            sum -= arr1[i] * 0.5;
            fsum *= 0.9f;
            isum ^= modifier;
        } else if (i % 13 == 0) {
            /* Another basic block */
            sum += arr2[i] * 2.0;
            isum |= 0x7F;
        }
        
        /* Memory operations with potential aliasing */
        arr1[i] = sum * 0.1;
        arr2[i] = temp * 0.8;
        
        /* Complex arithmetic mix */
        double div_op = (i % 3 == 0) ? 3.14159 : 2.71828;
        sum = sum / div_op + (double)isum * 0.001;
        
        /* Use volatile to force memory operations */
        farr[i % 256] = fsum;
        iarr[i] = isum + global_seed;
        
        /* Inline assembly to create scheduling complexity */
        asm volatile("" : "+r" (isum) : : "memory");
    }
    
    return sum + (double)fsum + (double)isum;
}

/* Another hot function to increase scheduling regions */
static inline int process_data(int *data, float *fdata, int count) {
    int result = 0;
    float accum = 0.0f;
    
    for (int i = 0; i < count; i++) {
        /* Multiple dependencies */
        result = (result * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Floating-point operations */
        accum = accum * 0.95f + fdata[i];
        
        /* Conditional with division (expensive) */
        if (result % 11 == 0) {
            accum /= 1.1f;
            data[i] = result >> 16;
        } else {
            accum *= 1.05f;
            data[i] = result & 0xFFFF;
        }
        
        /* Mix operations */
        fdata[i] = accum + (float)(i % 100);
        
        /* Memory barrier */
        asm volatile("" : : : "memory");
    }
    
    return result + (int)accum;
}

int main(void) {
    /* Allocate and initialize arrays */
    double *arr1 = (double*)malloc(SIZE * sizeof(double));
    double *arr2 = (double*)malloc(SIZE * sizeof(double));
    float *farr = (float*)malloc(256 * sizeof(float));
    int *iarr = (int*)malloc(SIZE * sizeof(int));
    int *data = (int*)malloc(SIZE * sizeof(int));
    float *fdata = (float*)malloc(SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !farr || !iarr || !data || !fdata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = (double)(i * 1.1);
        arr2[i] = (double)(i * 0.7);
        iarr[i] = i * 3;
        data[i] = i;
        fdata[i] = (float)i * 0.3f;
    }
    for (int i = 0; i < 256; i++) {
        farr[i] = (float)(i * 0.5);
    }
    
    double total = 0.0;
    int checksum = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < ITERATIONS / SIZE + 1; iter++) {
        /* Vary parameters to prevent optimization */
        int start = iter % 10;
        int end = SIZE - (iter % 20);
        int modifier = iter * 7;
        
        /* Main computation - should trigger selective scheduling */
        double result = compute_loop(arr1, arr2, farr, iarr, 
                                    start, end, modifier);
        total += result;
        
        /* Secondary computation */
        int res = process_data(data, fdata, SIZE - (iter % 100));
        checksum ^= res;
        
        /* Modify global to prevent loop invariant removal */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Final computation to use results */
    double final_sum = total;
    for (int i = 0; i < SIZE; i++) {
        final_sum += arr1[i] * 0.01;
        final_sum += arr2[i] * 0.02;
        checksum ^= iarr[i];
        checksum += data[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result: %f\nChecksum: %d\n", final_sum, checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(iarr);
    free(data);
    free(fdata);
    
    return 0;
}
