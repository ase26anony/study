#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Function with complex loop to engage selective scheduling */
int process_data(int n, int threshold, float float_thresh) {
    /* Arrays with mixed types */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Loop-carried dependency variables */
    int int_acc = 0;
    float float_acc = 0.0f;
    
    /* Volatile variable to prevent compile-time optimization */
    volatile int volatile_n = n;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < volatile_n; i++) {
            /* Loop-carried integer dependency */
            int_acc += arr1[i] * arr2[i-1];
            
            /* Loop-carried floating-point dependency */
            float_acc = farr1[i] * 0.9f + float_acc * 0.1f;
            
            /* Mixed-type operations with bitwise */
            int temp = (arr1[i] & 0xFF) + (int)(float_acc * 10);
            
            /* Conditional control flow inside loop */
            if (farr1[i] > float_thresh) {
                /* Floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i]));
                
                /* Integer operation inside conditional */
                arr1[i] = temp ^ (arr2[i] & 0xF);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1];
                arr1[i] = temp | (arr2[i] & 0xF0);
            }
            
            /* More complex indexing */
            int idx = (i * 2) % SIZE;
            if (idx > 0) {
                /* Cross-iteration memory dependency */
                arr2[idx] = arr1[i] + arr2[idx-1];
            }
            
            /* Another conditional with data-dependent condition */
            if (int_acc > threshold) {
                /* Reset accumulator partially */
                int_acc = int_acc / 2;
                farr1[i] = farr1[i] * 2.0f - 1.0f;
            }
            
            /* Floating-point to integer conversion */
            arr1[i] += (int)(farr2[i] * 100);
        }
        
        /* Small computation between outer iterations */
        for (int i = 0; i < 10; i++) {
            arr1[i] = arr1[i] ^ arr2[SIZE - i - 1];
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < volatile_n; i++) {
        checksum += arr1[i] + (int)farr2[i];
    }
    
    return checksum;
}

/* Another function with different pattern */
float process_floats(int n, float* output) {
    float acc = 0.0f;
    volatile int limit = n;
    
    for (int i = 0; i < limit; i++) {
        float x = (float)i / n;
        
        /* Trigonometric operations create complex FPU usage */
        float sin_val = sinf(x * 3.14159f);
        float cos_val = cosf(x * 1.57079f);
        
        /* Conditional with floating comparison */
        if (sin_val > cos_val) {
            acc += sin_val * 2.0f;
            output[i] = sin_val;
        } else {
            acc -= cos_val * 0.5f;
            output[i] = cos_val;
        }
        
        /* Mix with integer operations */
        int int_part = (int)(acc * 1000);
        output[i] += (float)(int_part % 100) * 0.01f;
    }
    
    return acc;
}

int main(int argc, char** argv) {
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Use command line argument for non-constant loop bound */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = 500;
    }
    
    /* Volatile threshold to prevent constant propagation */
    volatile int threshold = rand() % 10000;
    volatile float float_thresh = (float)rand() / RAND_MAX;
    
    /* Call processing functions */
    int result1 = process_data(n, threshold, float_thresh);
    
    float float_output[SIZE];
    float result2 = process_floats(n, float_output);
    
    /* Use results to prevent optimization */
    printf("Integer checksum: %d\n", result1);
    printf("Float result: %f\n", result2);
    
    /* Additional computation using both results */
    int final_result = result1 + (int)(result2 * 1000);
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
