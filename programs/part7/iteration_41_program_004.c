#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Complex loop with mixed operations to engage selective scheduler */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;  /* Loop-carried dependency */
    int mask = 0xFF;
    
    for (int i = 1; i < n; ++i) {
        /* Multiple memory accesses with non-trivial indexing */
        int idx1 = i;
        int idx2 = i - 1;  /* Creates dependency chain */
        int idx3 = (2 * i) % n;
        
        /* Loop-carried accumulation */
        acc += arr1[idx1] * arr2[idx2];
        
        /* Mixed integer/floating-point operations */
        float temp = farr1[idx1] * 2.0f - farr2[idx2];
        
        /* Conditional control flow with data-dependent condition */
        if (temp > 0.5f && (arr1[idx1] & mask) > threshold) {
            /* Complex floating-point operation */
            farr2[idx1] = sqrtf(fabsf(temp)) + (float)acc * 0.01f;
            
            /* Integer operation using floating-point result */
            arr2[idx1] = (arr2[idx2] & ~mask) | ((int)farr2[idx1] & mask);
        } else {
            /* Alternative computation path */
            farr2[idx1] = temp * 0.5f;
            arr2[idx1] = arr1[idx3] ^ arr2[idx2];
        }
        
        /* More mixed operations */
        arr1[idx1] = (arr1[idx1] * 3 + arr2[idx2]) / 2;
        
        /* Bitwise operation with floating-point conversion */
        int bits = *(int*)&farr2[idx1];
        arr1[idx1] ^= (bits >> 16) & 0xFFFF;
    }
}

/* Outer wrapper to create scheduling region */
void repeated_computation(int iterations, int size) {
    /* Dynamic allocation prevents compile-time optimization */
    int *arr1 = malloc(size * sizeof(int));
    int *arr2 = malloc(size * sizeof(int));
    float *farr1 = malloc(size * sizeof(float));
    float *farr2 = malloc(size * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < size; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 10.0f - 5.0f;
        farr2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
    
    /* Volatile variable to prevent constant propagation */
    volatile int dynamic_threshold = 128;
    
    /* Outer loop gives scheduler repeated region to analyze */
    for (int iter = 0; iter < iterations; ++iter) {
        /* Modify threshold slightly each iteration */
        int threshold = dynamic_threshold + (iter % 64);
        
        /* Call the complex kernel */
        compute_kernel(arr1, arr2, farr1, farr2, size, threshold);
        
        /* Cross-iteration dependency */
        arr1[0] = (arr1[size-1] + arr2[0]) % 1000;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < size; ++i) {
        checksum += arr1[i] + arr2[i] + (int)(farr1[i] * 100) + (int)(farr2[i] * 100);
    }
    
    printf("Checksum: %lld\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
}

int main(int argc, char *argv[]) {
    /* Seed RNG for non-deterministic behavior */
    srand(time(NULL));
    
    /* Use command-line argument or default to prevent constant folding */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Volatile to ensure compiler can't optimize away */
    volatile int iterations = 3;
    
    /* Perform computation with dynamic parameters */
    repeated_computation(iterations, n);
    
    return 0;
}
