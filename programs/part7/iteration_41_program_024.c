#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int get_value(int x) __attribute__((noinline));

int get_value(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

int main(int argc, char *argv[]) {
    /* Use volatile and argc to prevent compile-time optimization */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n <= 0 || n > SIZE) n = 500;
    
    /* Initialize arrays with mixed data types */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    int total_sum = 0;
    float float_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Loop-carried dependency variables */
        int int_acc = get_value(outer);
        float float_carry = (float)outer * 0.1f;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            int_acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            int temp = (arr1[i] & 0xFF) | (arr2[i] >> 3);
            float ftemp = (float)temp * 0.01f;
            
            /* Memory access with non-linear indexing */
            int idx = (i * 7) % SIZE;
            float_carry = farr1[idx] * 0.9f + float_carry * 0.1f;
            
            /* Conditional control flow inside loop */
            if (farr1[i] > 0.5f) {
                /* Floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i])) + float_carry;
                
                /* Integer operation inside conditional */
                arr1[i] = (arr1[i] ^ arr2[i]) + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = powf(fabsf(farr1[i]), 1.5f) - float_carry;
                arr1[i] = (arr1[i] & arr2[i]) | (int)(farr2[i] * 50.0f);
            }
            
            /* Additional dependency chain */
            if (i % 4 == 0) {
                int_acc -= (int)(farr2[i] * 10.0f);
                float_acc += farr2[i] * 0.01f;
            }
            
            /* Cross-iteration memory dependency */
            arr2[i] = arr1[i-1] + (int_acc % 100);
            
            /* Bitwise operation mixed with float */
            int mask = (int)(float_acc * 1000.0f) & 0x3FF;
            arr1[i] = (arr1[i] ^ mask) + (i * 17);
        }
        
        /* Use results to prevent dead code elimination */
        total_sum += int_acc + (int)(float_acc * 1000.0f);
        
        /* Shuffle data for next iteration */
        for (int i = 0; i < n; i++) {
            arr1[i] = (arr1[i] * 13) % 1000;
            farr1[i] = sinf(farr1[i] * 0.01f);
        }
    }
    
    /* Final checksum calculation */
    long long final_sum = 0;
    for (int i = 0; i < n; i++) {
        final_sum += arr1[i] + arr2[i] + (int)(farr1[i] * 100.0f) + (int)(farr2[i] * 100.0f);
    }
    
    printf("Result: %lld (total_sum: %d)\n", final_sum, total_sum);
    return 0;
}
