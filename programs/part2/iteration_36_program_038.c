/* sel-sched-test.c - Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Use volatile to prevent optimization */
static volatile int global_seed = 42;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr, double *darr,
                                int n, int iter) {
    int i;
    int local_sum = iter;
    float f_acc = 1.0f + iter * 0.1f;
    double d_acc = 2.0 + iter * 0.05;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = 0; i < n; i++) {
        /* Carried dependency on local_sum */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Independent floating point operations */
        f_acc = f_acc * 1.234567f + (float)arr1[i] * 0.001f;
        d_acc = d_acc / 1.0001 + (double)arr2[i % 16] * 0.0001;
        
        /* Memory operations with potential aliasing */
        arr1[i] = local_sum + (int)(f_acc * 100.0f);
        arr2[i % 16] = arr2[i % 16] ^ local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Different operation mix in this path */
            f_acc = f_acc - 0.5f;
            d_acc = d_acc * 1.5;
            arr1[i] = arr1[i] >> 2;
        } else if (i % 13 == 0) {
            /* Another basic block */
            d_acc = d_acc + (double)arr1[i] * 0.01;
            f_acc = f_acc / 1.1f;
        }
        
        /* More arithmetic diversity */
        if (i % 3 == 0) {
            farr[i % 8] = f_acc * 2.0f;
            darr[i % 8] = d_acc / 3.0;
        }
        
        /* Additional integer operations */
        int temp = (arr1[i] * 3) / (local_sum + 1);
        arr2[i % 16] = arr2[i % 16] + temp;
        
        /* Use volatile to force memory operations */
        global_seed = global_seed ^ (local_sum & 0xFF);
    }
    
    /* Store final results */
    if (n > 0) {
        arr1[0] = local_sum;
        farr[0] = f_acc;
        darr[0] = d_acc;
    }
}

/* Another hot function to encourage inlining */
static inline int process_data(int *data, float *fdata, double *ddata, int size) {
    int sum = 0;
    int temp_arr[16];
    int i;
    
    /* Initialize temp array */
    for (i = 0; i < 16; i++) {
        temp_arr[i] = i * global_seed;
    }
    
    /* Call compute_loop multiple times */
    for (i = 0; i < 4; i++) {
        compute_loop(data + i * 64, temp_arr, 
                     fdata + i * 8, ddata + i * 8,
                     size / 4, i);
    }
    
    /* Compute checksum */
    for (i = 0; i < size; i++) {
        sum = sum ^ data[i];
        sum = sum + (int)(fdata[i % 8] * 100.0f);
        sum = sum ^ (int)(ddata[i % 8] * 1000.0);
    }
    
    return sum;
}

int main(void) {
    const int DATA_SIZE = 256;
    int *int_data;
    float *float_data;
    double *double_data;
    int i, result;
    
    /* Allocate and initialize arrays */
    int_data = (int*)malloc(DATA_SIZE * sizeof(int));
    float_data = (float*)malloc(32 * sizeof(float));
    double_data = (double*)malloc(32 * sizeof(double));
    
    if (!int_data || !float_data || !double_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (i = 0; i < DATA_SIZE; i++) {
        int_data[i] = i * 3 + 1;
    }
    for (i = 0; i < 32; i++) {
        float_data[i] = i * 0.7f;
        double_data[i] = i * 1.3;
    }
    
    /* Perform computation - this should be the hot region */
    result = 0;
    for (i = 0; i < 100; i++) {
        result = result ^ process_data(int_data, float_data, double_data, DATA_SIZE);
        
        /* Modify data slightly each iteration */
        int_data[i % DATA_SIZE] = int_data[i % DATA_SIZE] + i;
        float_data[i % 32] = float_data[i % 32] * 1.01f;
        double_data[i % 32] = double_data[i % 32] / 1.01;
    }
    
    /* Final checksum */
    for (i = 0; i < DATA_SIZE; i++) {
        result = result ^ int_data[i];
    }
    for (i = 0; i < 32; i++) {
        result = result ^ (int)(float_data[i] * 100.0f);
        result = result ^ (int)(double_data[i] * 100.0);
    }
    
    printf("Result checksum: %d\n", result);
    printf("Global seed: %d\n", global_seed);
    
    /* Cleanup */
    free(int_data);
    free(float_data);
    free(double_data);
    
    return 0;
}
