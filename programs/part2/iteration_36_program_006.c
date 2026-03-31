/* Program to trigger selective scheduler debugging output */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Force memory dependencies with aliasing */
static inline void compute_loop(int *restrict arr1, int *arr2, 
                                float *restrict farr, double *darr,
                                int start, int end, int seed) {
    volatile int dep = seed; /* Volatile to prevent optimization */
    float fdep = (float)seed;
    double ddep = (double)seed;
    
    for (int i = start; i < end; i++) {
        /* Create carried dependencies */
        dep = dep * 1103515245 + 12345;
        fdep = fdep * 1.5f + (float)arr1[i % SIZE];
        ddep = ddep * 1.7 + (double)arr2[i % SIZE];
        
        /* Multiple independent operations */
        int temp1 = arr1[i % SIZE] * 3;
        int temp2 = arr2[i % SIZE] / 2;
        float ftemp = farr[i % SIZE] * 2.0f;
        double dtemp = darr[i % SIZE] / 1.3;
        
        /* Conditional branch creating multiple basic blocks */
        if (i % 7 == 0) {
            arr1[i % SIZE] = temp1 + dep;
            farr[i % SIZE] = ftemp + fdep;
        } else if (i % 13 == 0) {
            arr2[i % SIZE] = temp2 - dep;
            darr[i % SIZE] = dtemp - ddep;
        } else {
            /* Mixed operations with memory aliasing */
            arr1[i % SIZE] = (temp1 ^ temp2) | dep;
            arr2[i % SIZE] = (temp1 & temp2) ^ dep;
            farr[i % SIZE] = ftemp * fdep;
            darr[i % SIZE] = dtemp + ddep;
        }
        
        /* Additional floating point operations */
        if (i % 19 == 0) {
            fdep = fdep / 1.1f;
            ddep = ddep * 0.9;
        }
        
        /* Inline assembly with memory clobber to prevent optimization */
        asm volatile("" : : "r"(dep), "r"(fdep), "r"(ddep) : "memory");
    }
}

/* Another hot function with different pattern */
static inline void process_data(int *data, float *fdata, int n, int mod) {
    volatile int acc = mod;
    volatile float facc = (float)mod;
    
    for (int i = 0; i < n; i++) {
        /* Complex dependency chain */
        acc = (acc * 1664525 + 1013904223) % 65536;
        facc = facc * 1.618034f - (float)(i % 256);
        
        /* Memory operations with potential aliasing */
        int idx = (i * 17) % n;
        data[idx] = data[idx] ^ acc;
        fdata[idx] = fdata[idx] + facc;
        
        /* Division operation (expensive) */
        if (i % 23 == 0) {
            data[idx] = data[idx] / (acc + 1);
            fdata[idx] = fdata[idx] / (facc + 1.0f);
        }
        
        /* Additional control flow */
        switch (i % 5) {
            case 0: data[idx] += 1; break;
            case 1: data[idx] -= 2; break;
            case 2: data[idx] *= 3; break;
            case 3: data[idx] /= 4; break;
            default: data[idx] ^= 5; break;
        }
    }
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    
    if (!arr1 || !arr2 || !farr || !darr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 + 2;
        farr[i] = (float)i * 1.5f;
        darr[i] = (double)i * 2.5;
    }
    
    int checksum = 0;
    
    /* Call hot functions multiple times to create scheduling regions */
    for (int iter = 0; iter < 10; iter++) {
        compute_loop(arr1, arr2, farr, darr, 0, ITERATIONS, iter * 1000);
        process_data(arr1, farr, SIZE, iter * 2000);
    }
    
    /* Compute checksum to prevent optimization */
    for (int i = 0; i < SIZE; i++) {
        checksum ^= arr1[i];
        checksum ^= arr2[i];
        checksum ^= (int)farr[i];
        checksum ^= (int)darr[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}
