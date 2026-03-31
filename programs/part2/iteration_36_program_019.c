/* Program to trigger selective scheduler debug output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization */
static volatile int global_counter = 0;

/* Function with memory aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr, double *darr, 
                                int start, int end) {
    int i;
    float ftemp = 1.0f;
    double dtemp = 1.0;
    
    /* Hot loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        int idx = (i * 3) % SIZE;
        arr1[idx] = arr1[idx] + arr2[i % SIZE] * 2;
        
        /* Floating-point operations */
        ftemp = ftemp * 1.0001f + farr[i % SIZE];
        dtemp = dtemp / 1.000001 + darr[i % SIZE];
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            /* Complex operation in taken branch */
            arr1[idx] = arr1[idx] ^ (arr2[(i + 1) % SIZE] | 0x3F);
            ftemp = ftemp - 0.5f;
        } else if (i % 13 == 0) {
            /* Another branch with different operations */
            dtemp = dtemp * 0.9999;
            arr2[i % SIZE] = arr1[idx] >> 2;
        }
        
        /* Memory operations with potential aliasing */
        arr2[(i + 3) % SIZE] = arr1[idx] + global_counter;
        
        /* Mix of arithmetic operations */
        farr[i % SIZE] = ftemp * 2.0f - 1.0f;
        darr[i % SIZE] = dtemp + (i % 100) * 0.01;
        
        /* Additional integer math to increase instruction count */
        int temp = arr1[idx] * 3;
        temp = temp / (arr2[i % SIZE] + 1);
        arr1[idx] = temp % 1000;
        
        /* Inline assembly to create memory clobber */
        asm volatile("" : : "r"(arr1), "r"(arr2) : "memory");
    }
    
    /* Store results to prevent dead code elimination */
    farr[0] = ftemp;
    darr[0] = dtemp;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(int *restrict a, int *b, 
                                 float *restrict f, double *d,
                                 int chunk_size) {
    for (int j = 0; j < chunk_size; j += 64) {
        int end = (j + 64 < chunk_size) ? j + 64 : chunk_size;
        compute_loop(a, b, f, d, j, end);
        
        /* Additional operations between chunks */
        if (j % 128 == 0) {
            double sum = 0.0;
            for (int k = 0; k < 8; k++) {
                sum += d[k] * f[k];
            }
            global_counter += (int)sum;
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    float *farray = (float*)malloc(SIZE * sizeof(float));
    double *darray = (double*)malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !farray || !darray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3 + 1;
        array2[i] = i * 5 + 2;
        farray[i] = (float)i * 0.1f + 0.5f;
        darray[i] = (double)i * 0.01 + 0.25;
    }
    
    /* Perform multiple iterations to create hot loop */
    int checksum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call the compute function multiple times */
        process_chunk(array1, array2, farray, darray, SIZE);
        
        /* Update checksum to prevent optimization */
        checksum ^= array1[iter % SIZE];
        checksum ^= array2[iter % SIZE];
        
        /* Modify global counter to create dependencies */
        global_counter += (iter % 100);
        
        /* Occasionally reset some values */
        if (iter % 1000 == 0) {
            array1[0] = iter;
            array2[0] = iter * 2;
        }
    }
    
    /* Final computation to ensure all code is used */
    double final_result = 0.0;
    for (int i = 0; i < SIZE; i++) {
        final_result += array1[i] * 0.01;
        final_result += array2[i] * 0.02;
        final_result += farray[i];
        final_result += darray[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    printf("Final result: %f\n", final_result);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(farray);
    free(darray);
    
    return 0;
}
