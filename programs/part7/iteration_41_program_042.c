#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int *arr1, int *arr2, float *farr1, float *farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            float temp = (float)acc / (i + 1);
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(farr1[i] * temp);
                
                /* Bitwise operation combined with float conversion */
                arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1];
                arr1[i] = arr1[i] | (arr2[i] << 2);
            }
            
            /* Additional loop-carried float dependency */
            f_acc += farr2[i] * 0.1f;
            
            /* Complex array indexing */
            int idx = (i * 3) % n;
            arr2[idx] = arr1[i] + arr2[(i-1) % n];
            
            /* More mixed operations */
            farr1[i] = f_acc + (float)(arr1[i] % 100);
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
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
    }
    
    /* Calculate checksum to ensure computation isn't optimized away */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    return 0;
}

/* Implementation to prevent external reference */
int use_result(int val) {
    return val % 100;
}
