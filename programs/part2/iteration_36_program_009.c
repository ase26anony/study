/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with memory aliasing to create dependencies */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                               float *restrict farr, double *darr,
                               int n, int iter) {
    int i;
    float f_acc = 1.0f;
    double d_acc = 1.0;
    int int_acc = global_seed + iter;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Integer operations with carried dependency */
        int_acc = int_acc * 1103515245 + 12345;
        
        /* Floating point operations */
        f_acc = f_acc * 1.01f + (float)int_acc * 0.001f;
        d_acc = d_acc / 1.0001 + (double)int_acc * 0.0001;
        
        /* Memory operations with potential aliasing */
        arr1[i] = int_acc + (int)f_acc;
        arr2[i % 16] = arr2[i % 16] + int_acc;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            farr[i % 8] = f_acc * 2.0f;
            d_acc = d_acc - 0.5;
        } else if (i % 13 == 0) {
            darr[i % 8] = d_acc * 3.0;
            f_acc = f_acc + 1.0f;
        } else {
            /* Mix of operations in the else path */
            int_acc = int_acc ^ (i * 0x5A5A5A5A);
            f_acc = f_acc - 0.1f;
        }
        
        /* Additional arithmetic to increase instruction diversity */
        int_acc = (int_acc << 3) | (int_acc >> 29);  /* rotate */
        f_acc = f_acc + (float)(i % 256) * 0.01f;
        d_acc = d_acc * (1.0 + (double)(i % 128) * 0.0001);
        
        /* Memory store with dependency on previous load */
        if (i > 0) {
            arr1[i] = arr1[i] + arr2[(i-1) % 16];
        }
    }
    
    /* Store final accumulators */
    arr1[0] = int_acc;
    farr[0] = f_acc;
    darr[0] = d_acc;
}

/* Another hot function to encourage inlining and scheduling */
static inline int process_data(int *data, float *fdata, int size, int offset) {
    int sum = offset;
    float fsum = 0.0f;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex dependency chain */
        int val = data[i];
        val = val * 636413622 + 1013904223;
        
        /* Floating point with type conversion */
        fdata[i] = (float)val * 0.001f + fsum;
        fsum = fdata[i] * 0.99f;
        
        /* Conditional with arithmetic */
        if (val % 11 == 0) {
            sum += val >> 4;
            fsum = fsum + 2.0f;
        } else if (val % 19 == 0) {
            sum -= val & 0xFF;
            fsum = fsum - 1.0f;
        }
        
        /* More operations to create scheduling opportunities */
        sum = sum ^ (val * 0x12345678);
        data[i] = sum;
        
        /* Inline asm to prevent optimization and add complexity */
        asm volatile("" : "+r"(sum) : : "memory");
    }
    
    return sum;
}

int main(void) {
    const int ARRAY_SIZE = 1024;
    const int ITERATIONS = 100;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *arr2 = (int*)malloc(16 * sizeof(int));
    float *farr = (float*)malloc(ARRAY_SIZE * sizeof(float));
    double *darr = (double*)malloc(ARRAY_SIZE * sizeof(double));
    int *data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *fdata = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !farr || !darr || !data || !fdata) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i;
        farr[i] = (float)i * 0.5f;
        darr[i] = (double)i * 0.25;
        data[i] = i * 3;
        fdata[i] = (float)i * 0.33f;
    }
    
    for (int i = 0; i < 16; i++) {
        arr2[i] = i * 7;
    }
    
    int checksum = 0;
    
    /* Perform multiple iterations to create hot loops */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call the hot loop function - should be inlined */
        compute_loop(arr1, arr2, farr, darr, ARRAY_SIZE, iter);
        
        /* Process data with another hot function */
        int result = process_data(data, fdata, ARRAY_SIZE, iter);
        checksum ^= result;
        
        /* Modify global to create external dependency */
        global_seed = (global_seed * 1664525 + 1013904223) & 0x7FFFFFFF;
    }
    
    /* Compute final checksum to prevent optimization */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= *(int*)&farr[i];  /* Treat float as int for checksum */
        checksum ^= data[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    free(data);
    free(fdata);
    
    return 0;
}
