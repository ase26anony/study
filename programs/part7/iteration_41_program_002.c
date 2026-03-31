#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Complex loop with mixed operations to engage selective scheduler */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;  /* Loop-carried dependency */
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int iter = 0; iter < 3; ++iter) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-linear indexing */
            int idx1 = i;
            int idx2 = 2 * i;
            int idx3 = i - 1;
            
            /* Loop-carried integer dependency */
            acc += arr1[idx1] * arr2[idx3];
            
            /* Mixed integer/float operations */
            float temp = farr1[idx1] * 1.5f + (float)acc * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (temp > (float)threshold) {
                /* Complex floating-point operation */
                farr2[idx1] = sqrtf(fabsf(temp)) + f_acc;
                
                /* Bitwise operation mixed with float conversion */
                arr1[idx1] = (arr1[idx1] & 0xFF) | ((int)farr2[idx1] << 8);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = powf(fabsf(temp), 1.5f) - f_acc;
                arr1[idx1] = (arr1[idx1] ^ 0xAA) + (int)(farr2[idx1] * 100.0f);
            }
            
            /* Additional loop-carried float dependency */
            f_acc = f_acc * 0.99f + farr2[idx3] * 0.1f;
            
            /* More mixed operations */
            arr2[idx1] = (arr1[idx1] + arr2[idx3]) * (i & 0xF);
            
            /* Another conditional with arithmetic */
            if ((arr1[idx1] + arr2[idx1]) % 7 == 0) {
                farr1[idx1] = sinf(farr2[idx1]) * cosf(f_acc);
            }
        }
        
        /* Slight variation between outer iterations */
        threshold = (threshold + iter) & 0x3F;
    }
}

int main(int argc, char **argv) {
    /* Use argc to prevent compile-time constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    
    /* Ensure n is reasonable */
    if (n < 100) n = 100;
    if (n > 10000) n = 10000;
    
    /* Volatile to prevent optimization */
    volatile int threshold = 50;
    
    /* Allocate arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
    
    /* Perform computation */
    compute_kernel(arr1, arr2, farr1, farr2, n, threshold);
    
    /* Calculate checksum to prevent dead code elimination */
    long long checksum = 0;
    double f_checksum = 0.0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Float checksum: %f\n", f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
