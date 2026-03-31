/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Function with potential aliasing */
static inline void compute_loop(int *restrict a, int *b, float *c, 
                               double *d, int n, int seed) {
    volatile int dep1 = seed;  /* Volatile to prevent optimization */
    volatile float dep2 = seed * 0.5f;
    volatile double dep3 = seed * 0.25;
    
    for (int i = 0; i < n; i++) {
        /* Create carried dependencies */
        dep1 = dep1 * 1103515245 + 12345;
        dep2 = dep2 * 1.5f + (float)dep1 * 0.01f;
        dep3 = dep3 * 1.25 + (double)dep2 * 0.001;
        
        /* Multiple independent operations */
        int temp1 = a[i] * 3 + b[i % 16];
        float temp2 = c[i] * 2.0f / (dep2 + 1.0f);
        double temp3 = d[i] * 1.5 / (dep3 + 1.0);
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            temp1 += dep1;
            temp2 *= 1.1f;
            temp3 -= 0.5;
        } else if (i % 13 == 0) {
            temp1 -= dep1 >> 2;
            temp2 /= 1.2f;
            temp3 += 0.25;
        } else {
            temp1 ^= 0x55AA55AA;
            temp2 = temp2 + 0.5f - temp2 * 0.1f;
            temp3 = temp3 * 0.9 + 0.1;
        }
        
        /* Memory operations with potential aliasing */
        a[i] = temp1 + (int)temp2 + (int)temp3;
        b[i % 16] = b[(i + 1) % 16] ^ temp1;
        c[i] = temp2 * 0.9f + c[(i + 4) % n] * 0.1f;
        d[i] = temp3 * 0.8 + d[(i + 8) % n] * 0.2;
        
        /* Additional arithmetic diversity */
        if (i % 19 == 0) {
            /* Integer division */
            a[i] /= (dep1 & 0xFF) + 1;
            /* Floating division */
            c[i] /= (dep2 + 1.0f);
            d[i] /= (dep3 + 1.0);
        }
    }
}

/* Another hot function to encourage inlining */
static inline void process_chunk(int *restrict arr1, int *arr2, 
                                float *arr3, double *arr4, int start, int end) {
    for (int i = start; i < end; i++) {
        /* Complex expression with mixed operations */
        arr1[i] = (arr1[i] * 3 + arr2[i % 32]) ^ (arr1[i] >> 4);
        arr3[i] = arr3[i] * 1.7f - arr3[(i + 3) % SIZE] * 0.3f;
        arr4[i] = arr4[i] * 1.6 - arr4[(i + 5) % SIZE] * 0.4;
        
        /* More conditional logic */
        switch (i % 5) {
            case 0: arr1[i] += 100; break;
            case 1: arr1[i] -= 50; break;
            case 2: arr1[i] |= 0xFF; break;
            case 3: arr1[i] &= 0x7F; break;
            case 4: arr1[i] ^= arr2[i % 32]; break;
        }
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(32 * sizeof(int));
    float *array3 = (float*)malloc(SIZE * sizeof(float));
    double *array4 = (double*)malloc(SIZE * sizeof(double));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        array1[i] = i * 3;
        array3[i] = i * 0.7f;
        array4[i] = i * 0.3;
    }
    for (int i = 0; i < 32; i++) {
        array2[i] = i * 5;
    }
    
    /* Perform multiple calls to create scheduling regions */
    uint64_t checksum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call the hot loop function */
        compute_loop(array1, array2, array3, array4, SIZE, iter);
        
        /* Call another hot function */
        process_chunk(array1, array2, array3, array4, 0, SIZE);
        
        /* Update checksum to prevent optimization */
        checksum ^= (uint64_t)array1[iter % SIZE];
        checksum += (uint64_t)(array3[iter % SIZE] * 1000);
        checksum ^= (uint64_t)(array4[iter % SIZE] * 10000);
    }
    
    /* Final computation to use results */
    int final_result = 0;
    for (int i = 0; i < SIZE; i++) {
        final_result ^= array1[i];
        final_result += (int)(array3[i] * 100);
        final_result ^= (int)(array4[i] * 1000);
    }
    
    /* Mix in the checksum */
    final_result ^= (int)(checksum & 0xFFFFFFFF);
    final_result ^= (int)(checksum >> 32);
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
