#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, int n) {
    int acc = 0;
    volatile int threshold = 500; /* Prevent constant propagation */
    
    for (int i = 1; i < n; ++i) {
        /* Loop-carried dependency */
        acc += arr1[i] * arr2[i-1];
        
        /* Mixed integer operations with bitwise */
        int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
        
        /* Floating-point computation */
        float fval = farr1[i] * 2.0f - farr1[i-1];
        
        /* Conditional with data-dependent branch */
        if (fval > 0.5f && (temp % 7) != 0) {
            farr2[i] = sqrtf(fabsf(fval));
            
            /* More mixed operations */
            arr1[i] = (int)(farr2[i] * 100.0f) ^ temp;
        } else {
            farr2[i] = fval * 0.5f;
            arr1[i] = temp + (int)fval;
        }
        
        /* Additional dependency chain */
        arr2[i] = arr2[i-1] + (int)(sinf(farr2[i]) * 10.0f);
        
        /* Complex indexing pattern */
        if (i * 2 < n) {
            farr1[i*2] += farr2[i] * 0.1f;
        }
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

/* Outer wrapper with multiple loops */
void process_data(int iterations, int data_size) {
    /* Dynamically allocate to avoid stack overflow */
    int *arr1 = malloc(data_size * sizeof(int));
    int *arr2 = malloc(data_size * sizeof(int));
    float *farr1 = malloc(data_size * sizeof(float));
    float *farr2 = malloc(data_size * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < data_size; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    /* Outer loop to give scheduler repeated region */
    for (int iter = 0; iter < iterations; ++iter) {
        /* Modify parameters slightly each iteration */
        volatile int adjusted_size = data_size - (iter % 10);
        
        /* Call the complex kernel */
        compute_kernel(arr1, arr2, farr1, farr2, adjusted_size);
        
        /* Shuffle data between iterations */
        for (int i = 1; i < data_size; ++i) {
            arr1[i] = (arr1[i] + arr2[i-1]) % 1000;
            farr1[i] = farr1[i] * 0.9f + farr2[i-1] * 0.1f;
        }
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    int checksum = 0;
    for (int i = 0; i < data_size; ++i) {
        checksum += arr1[i] + (int)farr2[i];
    }
    printf("Checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Use command line argument for variable iteration count */
    int iterations = 3;
    int data_size = SIZE;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 3;
    }
    if (argc > 2) {
        data_size = atoi(argv[2]);
        if (data_size <= 100) data_size = SIZE;
    }
    
    /* Process with variable parameters */
    process_data(iterations, data_size);
    
    return 0;
}

/* Dummy function to prevent optimization */
int use_result(int x) {
    return x % 100;
}
