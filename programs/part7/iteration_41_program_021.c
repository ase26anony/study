#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Function with complex loop to engage selective scheduling */
void process_data(int n, float threshold, int* arr1, int* arr2, 
                  float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Reset accumulators */
        acc = 0;
        f_acc = 0.0f;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Loop-carried dependency on integer accumulator */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/floating-point operations */
            float temp = (float)acc * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > threshold) {
                /* Floating-point operation inside conditional */
                farr2[i] = sqrtf(fabsf(farr1[i]));
                
                /* Integer operation using floating-point result */
                arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 10.0f);
            } else {
                /* Alternative path with different operations */
                farr2[i] = farr1[i] * farr1[i-1];
                arr1[i] = arr1[i] | (arr2[i] << 2);
            }
            
            /* More mixed operations */
            f_acc += farr2[i] * 0.5f;
            
            /* Complex array indexing with non-linear pattern */
            int idx = (i * 7) % n;
            arr2[idx] = arr1[i] + arr2[(i * 3) % n];
            
            /* Additional floating-point operation */
            farr1[i] = f_acc * sinf((float)i * 0.01f);
        }
        
        /* Cross-iteration dependency between outer loop iterations */
        arr1[0] += acc;
        farr1[0] += f_acc;
    }
}

/* Another function with different pattern to increase scheduling complexity */
void transform_data(int n, int* arr1, int* arr2, float* farr1) {
    volatile int limit = n; /* Prevent constant propagation */
    
    for (int i = 0; i < limit; ++i) {
        /* Bitwise operations mixed with arithmetic */
        int val = arr1[i] ^ arr2[i];
        val = (val << 3) | (val >> 29); /* Rotation */
        
        /* Conditional with floating-point comparison */
        if ((float)val > farr1[i]) {
            arr1[i] = val % 1000;
        } else {
            arr1[i] = (arr1[i] + arr2[i]) * 2;
        }
        
        /* Memory access with stride */
        if (i + 8 < n) {
            arr2[i + 8] = arr1[i] - arr2[i];
        }
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    /* Volatile threshold to prevent constant folding */
    volatile float threshold = 0.5f;
    
    /* Process data multiple times */
    for (int iter = 0; iter < 2; ++iter) {
        process_data(n, threshold, arr1, arr2, farr1, farr2);
        transform_data(n, arr1, arr2, farr1);
        
        /* Modify threshold slightly each iteration */
        threshold += 0.1f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    return 0;
}
