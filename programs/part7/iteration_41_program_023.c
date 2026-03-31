#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, float threshold) {
    int acc = 0;  /* Loop-carried dependency */
    int temp;
    float ftemp;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        acc = 0;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency on acc */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/floating-point operations */
            ftemp = (float)acc * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > threshold) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i] + ftemp));
            } else {
                farr2[i] = farr1[i] * 0.5f;
            }
            
            /* Bitwise operations combined with floating-point */
            temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* More mixed operations with array dependencies */
            arr1[i] = temp + (int)(farr2[i] * 100.0f) + arr1[i-1];
            
            /* Additional floating-point computation */
            farr1[i] = farr2[i] * 0.8f + sinf((float)i * 0.01f);
            
            /* Another conditional with arithmetic */
            if (arr1[i] % 7 == 0) {
                arr2[i] = arr2[i] ^ 0x55;
            } else {
                arr2[i] = arr2[i] + i;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : SIZE;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Volatile variable to prevent constant propagation */
    volatile int volatile_n = n;
    
    /* Initialize arrays with pseudo-random data */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    /* Data-dependent threshold */
    float threshold = (float)(rand() % 100) / 100.0f;
    
    /* Perform computation */
    compute_kernel(arr1, arr2, farr1, farr2, volatile_n, threshold);
    
    /* Calculate checksum to ensure computation isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < volatile_n; i++) {
        checksum += arr1[i] + (int)farr1[i];
        checksum ^= arr2[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* External function definition to prevent optimization */
int use_result(int value) {
    return value % 256;
}
