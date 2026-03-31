/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Use volatile to prevent optimization */
static volatile int global_counter = 0;

/* Function with potential aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr1, double *darr,
                                int start, int end, int seed) {
    int i;
    int local_sum = seed;
    float fsum = seed * 0.5f;
    double dsum = seed * 0.25;
    
    /* Complex loop with multiple dependencies and operations */
    for (i = start; i < end; i++) {
        /* Integer operations with carried dependency */
        local_sum = local_sum * 1103515245 + 12345;
        
        /* Floating point operations */
        fsum = fsum * 1.5f + (i % 256) * 0.01f;
        dsum = dsum / 1.7 + (i % 128) * 0.001;
        
        /* Memory operations with potential aliasing */
        arr1[i] = local_sum + (int)fsum;
        arr2[i % 16] = arr2[i % 16] * 13 + local_sum;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            farr1[i % 32] = fsum * 2.0f;
            darr[i % 64] = dsum * 3.0;
            /* Additional operation in this path */
            local_sum += (i % 19);
        } else if (i % 13 == 0) {
            /* Another basic block */
            farr1[i % 32] = fsum / 2.0f;
            darr[i % 64] = dsum / 3.0;
            local_sum -= (i % 23);
        } else {
            /* Default path */
            farr1[i % 32] = fsum;
            darr[i % 64] = dsum;
        }
        
        /* More arithmetic diversity */
        if (i % 17 == 0) {
            /* Use inline assembly to create specific RTL patterns */
            asm volatile("" : "+r" (local_sum) : : "memory");
        }
        
        /* Mix operations */
        local_sum = (local_sum << 3) | (local_sum >> 29); /* rotate */
        fsum = fsum + (local_sum % 100) * 0.001f;
        dsum = dsum - (local_sum % 50) * 0.0001;
        
        /* Update global volatile to prevent dead code elimination */
        global_counter += (i & 1);
    }
    
    /* Store final results */
    arr1[0] = local_sum;
    farr1[0] = fsum;
    darr[0] = dsum;
}

/* Another hot function to encourage inlining */
static inline int process_chunk(int *restrict buf1, int *buf2,
                               float *restrict fbuf, double *dbuf,
                               int chunk_size, int base) {
    int i, result = 0;
    
    for (i = 0; i < chunk_size; i++) {
        /* Different computation pattern */
        int idx = (i + base) % chunk_size;
        buf1[idx] = buf1[idx] * 3 + buf2[i % 8];
        buf2[i % 8] = buf2[i % 8] ^ buf1[idx];
        
        /* Floating point with type conversion */
        fbuf[idx % 16] = (float)buf1[idx] * 0.25f + fbuf[idx % 16];
        dbuf[idx % 32] = (double)buf2[i % 8] * 0.125 + dbuf[idx % 32];
        
        /* Complex conditional */
        if ((buf1[idx] & 0xFF) > 128) {
            result += buf1[idx];
            fbuf[idx % 16] *= 1.1f;
        } else {
            result -= buf2[i % 8];
            dbuf[idx % 32] /= 1.05;
        }
        
        /* Additional operation to increase instruction count */
        result = (result << 1) | (result >> 31);
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(16 * sizeof(int));
    float *farr1 = (float*)malloc(32 * sizeof(float));
    double *darr = (double*)malloc(64 * sizeof(double));
    
    int i, j;
    int checksum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) arr1[i] = i;
    for (i = 0; i < 16; i++) arr2[i] = i * 2;
    for (i = 0; i < 32; i++) farr1[i] = i * 0.5f;
    for (i = 0; i < 64; i++) darr[i] = i * 0.25;
    
    /* Main computation - multiple calls to hot functions */
    for (j = 0; j < ITERATIONS / 1000; j++) {
        /* Call the compute_loop function multiple times with different parameters */
        compute_loop(arr1, arr2, farr1, darr, 0, SIZE / 2, j * 3);
        compute_loop(arr1, arr2, farr1, darr, SIZE / 2, SIZE, j * 7);
        
        /* Also call process_chunk to create more scheduling opportunities */
        checksum ^= process_chunk(arr1, arr2, farr1, darr, 256, j * 11);
        
        /* Additional computation to keep the loop hot */
        for (i = 0; i < 8; i++) {
            arr2[i] = arr2[i] * 17 + checksum;
            farr1[i] = farr1[i] * 1.3f - (checksum % 100) * 0.01f;
        }
    }
    
    /* Final checksum computation to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
    }
    for (i = 0; i < 16; i++) {
        checksum ^= arr2[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", checksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(darr);
    
    return 0;
}
