#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Prevent inlining to maintain control flow complexity */
__attribute__((noinline)) 
void process_loop(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;  /* Loop-carried dependency */
    volatile int threshold = 100;  /* Prevent constant propagation */
    
    for (int i = 1; i < n; ++i) {
        /* Complex addressing with multiple dependencies */
        int idx1 = i;
        int idx2 = i - 1;  /* Creates dependency chain */
        int idx3 = (i * 2) % n;
        
        /* Mixed integer operations with loop-carried dependency */
        int temp = arr1[idx1] * arr2[idx2] + acc;
        acc = (temp & 0x7FF) + (acc >> 3);  /* Non-linear update */
        
        /* Floating-point computation with conditional */
        float fval = farr1[idx1] * 2.0f - farr2[idx2];
        if (fval > 0.5f) {  /* Data-dependent branch */
            farr2[idx1] = sqrtf(fabsf(fval)) + 0.1f;
        } else {
            farr2[idx1] = fval * fval;
        }
        
        /* Cross-type conversion and operation */
        arr1[idx3] = (arr1[idx3] ^ 0x55) + (int)(farr2[idx1] * 10.0f);
        
        /* Additional memory access pattern */
        arr2[idx1] = arr2[idx2] + arr1[idx3] - threshold;
        
        /* Another conditional with bitwise operations */
        if ((arr1[idx1] & 0xF) > 8) {
            farr1[idx1] = (float)(arr1[idx1] % 17) / 16.0f;
        }
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 1000) n = 1000;
    }
    
    /* Allocate and initialize with random data */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr1 = (float*)malloc(n * sizeof(float));
    float* farr2 = (float*)malloc(n * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        arr1[i] = rand() % 256;
        arr2[i] = rand() % 256;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Outer loop to give scheduler repeated region */
    volatile int outer_iter = 3;
    int total_sum = 0;
    
    for (int iter = 0; iter < outer_iter; ++iter) {
        /* Modify inputs slightly each iteration */
        arr1[0] += iter;
        farr1[0] += (float)iter * 0.1f;
        
        process_loop(n, arr1, arr2, farr1, farr2);
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (int i = 0; i < n; ++i) {
        total_sum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    printf("Checksum: %d\n", total_sum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
