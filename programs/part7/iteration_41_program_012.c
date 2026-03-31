#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int external_helper(int x);

int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

void process_loop(int *arr1, int *arr2, float *farr1, float *farr2, 
                  int n, int threshold) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Reset accumulators */
        acc = external_helper(outer);
        f_acc = (float)acc / 1000.0f;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Loop-carried dependency on acc */
            int temp = arr1[i] * arr2[i-1];
            acc += temp;
            
            /* Mixed integer/float operations */
            float f_val = (float)temp / 256.0f;
            farr1[i] = f_val + f_acc;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f && (arr1[i] & 0x3) == 0) {
                /* Complex floating point operation */
                farr2[i] = sqrtf(fabsf(farr1[i])) + 
                          sinf((float)i * 0.01f);
                
                /* Integer operation using float result */
                arr2[i] = (int)(farr2[i] * 100.0f) & 0xFFF;
            } else {
                /* Alternative path with different operations */
                farr2[i] = powf(fabsf(farr1[i]), 1.5f);
                arr2[i] = (arr1[i] << 2) | (arr2[i-1] & 0xF);
            }
            
            /* More mixed operations */
            f_acc = farr1[i] * 0.9f + farr2[i] * 0.1f;
            
            /* Bitwise operations combined with arithmetic */
            arr1[i] = ((arr1[i] ^ arr2[i-1]) + acc) & 0xFFFF;
            
            /* Memory access with non-linear indexing */
            if (i * 2 < n) {
                arr1[i] += arr2[i * 2 % n];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n <= 0 || n > SIZE) n = SIZE;
    
    /* Initialize arrays */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 1000.0f;
        farr2[i] = (float)(rand() % 1000) / 1000.0f;
    }
    
    /* Volatile threshold to prevent constant propagation */
    volatile int threshold = rand() % 500 + 250;
    
    /* Process the data */
    process_loop(arr1, arr2, farr1, farr2, n, threshold);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Integer checksum: %lld\n", checksum);
    printf("Float checksum: %f\n", f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
