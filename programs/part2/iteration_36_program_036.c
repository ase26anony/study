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
                                float *restrict farr, double *darr, 
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
        fsum = fsum * 1.5f + (float)local_sum * 0.01f;
        dsum = dsum * 1.7 + (double)fsum * 0.005;
        
        /* Memory operations with potential aliasing */
        arr1[i % SIZE] = local_sum;
        arr2[i % SIZE] = local_sum ^ (int)fsum;
        
        /* More arithmetic with mixed types */
        float ftemp = (float)(i) * 0.3f;
        double dtemp = (double)(i % 17) * 0.7;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            ftemp *= 2.0f;
            dtemp /= 3.0;
            arr1[i % SIZE] += (int)(ftemp * dtemp);
        } else if (i % 13 == 0) {
            ftemp /= 1.5f;
            dtemp *= 2.5;
            arr2[i % SIZE] -= (int)(ftemp + dtemp);
        } else {
            ftemp += 1.0f;
            dtemp -= 0.5;
        }
        
        /* More operations to increase instruction mix */
        farr[i % SIZE] = ftemp + fsum * 0.1f;
        darr[i % SIZE] = dtemp + dsum * 0.05;
        
        /* Integer division/modulo - expensive operations */
        if (i % 11 == 0) {
            local_sum /= 3;
            fsum = fsum / 1.3f;
        }
        
        /* Bitwise operations */
        local_sum ^= (i << 3);
        local_sum |= (i % 255);
        
        /* Prevent loop invariant code motion */
        asm volatile("" : "+r" (local_sum), "+r" (fsum), "+r" (dsum));
    }
    
    /* Store results to prevent dead code elimination */
    arr1[0] = local_sum;
    farr[0] = fsum;
    darr[0] = dsum;
    
    /* Update global volatile to create side effect */
    global_counter += local_sum;
}

/* Another inline function to increase scheduling complexity */
static inline void process_chunk(int *restrict a, int *b, 
                                 float *restrict f, double *d,
                                 int chunk_size, int offset) {
    int i, j;
    
    /* Nested loops increase scheduling region size */
    for (j = 0; j < 3; j++) {
        for (i = 0; i < chunk_size; i++) {
            /* Complex expression with multiple dependencies */
            int idx = (i + offset) % SIZE;
            int val = a[idx] * 3 + b[idx] * 2;
            
            /* Floating point operations */
            float fval = f[idx] * 2.0f - (float)val * 0.01f;
            double dval = d[idx] * 1.5 + (double)fval * 0.02;
            
            /* Conditional store */
            if (val > 0) {
                a[idx] = val % 1000;
                f[idx] = fval + (float)(i % 10);
            } else {
                b[idx] = -val % 1000;
                d[idx] = dval - (double)(j % 5);
            }
            
            /* More arithmetic */
            f[idx] = f[idx] * 0.99f + 0.01f;
            d[idx] = d[idx] * 0.98 + 0.02;
            
            /* Memory barrier to prevent reordering across iterations */
            asm volatile("" ::: "memory");
        }
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i;
        arr2[i] = SIZE - i;
        farr[i] = (float)i * 0.5f;
        darr[i] = (double)i * 0.25;
    }
    
    int total_iterations = ITERATIONS;
    int chunk_size = SIZE / 4;
    
    /* Perform multiple calls to create scheduling regions */
    for (int iter = 0; iter < total_iterations / chunk_size; iter++) {
        /* Call the hot function multiple times with different parameters */
        compute_loop(arr1, arr2, farr, darr, 
                     iter * chunk_size, 
                     (iter + 1) * chunk_size,
                     iter * 17 + 123);
        
        /* Process the results further */
        process_chunk(arr1, arr2, farr, darr, chunk_size, iter * 7);
        
        /* Additional computation to increase pressure */
        for (int i = 0; i < chunk_size; i++) {
            int idx = (iter * 11 + i) % SIZE;
            arr1[idx] = (arr1[idx] * 3 - arr2[idx]) % 10000;
            farr[idx] = farr[idx] * 1.1f + (float)arr1[idx] * 0.001f;
            darr[idx] = darr[idx] * 1.05 + (double)arr2[idx] * 0.0005;
        }
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    float fchecksum = 0.0f;
    double dchecksum = 0.0;
    
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i];
        fchecksum += farr[i];
        dchecksum += darr[i];
        
        /* Mix checksums */
        checksum ^= (int)farr[i];
        checksum += (int)darr[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Checksums: int=%d, float=%.2f, double=%.2f\n", 
           checksum, fchecksum, dchecksum);
    printf("Global counter: %d\n", global_counter);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}
