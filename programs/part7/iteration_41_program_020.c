#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining and create more complex RTL */
extern int external_helper(int x);

/* Function with complex loop that should trigger selective scheduling */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    volatile int limit = n;  /* Prevent constant propagation */
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with mixed operations */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 8);
            temp = external_helper(temp);
            
            /* Floating-point operations */
            f_acc += farr1[i] * 1.5f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f && (acc & 1)) {
                farr2[i] = sqrtf(fabsf(farr1[i]));
                arr1[i] = (int)(farr2[i] * 100.0f) ^ temp;
            } else {
                farr2[i] = farr1[i] * 2.0f;
                arr1[i] = temp - (int)farr2[i];
            }
            
            /* More complex addressing with non-linear index */
            int idx = (i * 3) % SIZE;
            if (idx > 0) {
                arr2[idx] = arr1[i] + arr2[idx-1] + (int)(sinf(farr2[i]) * 10);
            }
            
            /* Another conditional with floating comparison */
            if (f_acc > 100.0f) {
                f_acc -= 50.0f;
                arr1[i] >>= 2;
            }
        }
        
        /* Reset accumulator partially */
        acc = acc % 1000;
    }
}

/* Simple external helper function */
int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays with random data */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Process arrays multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        
        /* Modify arrays slightly between iterations */
        for (int i = 0; i < n; i++) {
            arr1[i] += iter;
            farr1[i] += iter * 0.1f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    long long checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
        /* Mix in some bitwise operations */
        checksum ^= (long long)arr1[i] << (i % 16);
    }
    
    printf("Checksum: %lld, Float checksum: %f\n", checksum, f_checksum);
    
    return 0;
}
