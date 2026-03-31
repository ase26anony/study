#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;  /* Loop-carried dependency */
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            float temp = (float)acc * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f) {
                /* Non-trivial floating point operation */
                farr2[i] = sqrtf(fabsf(farr1[i] + temp));
            } else {
                farr2[i] = farr1[i] * 2.0f - temp;
            }
            
            /* Bitwise operations combined with float conversion */
            arr1[i] = (arr1[i] & 0xFF) | ((int)(farr2[i] * 100.0f) << 8);
            
            /* Complex array indexing with multiple dependencies */
            arr2[i] = arr1[i-1] + arr2[(i*2) % n] + (int)(farr1[(i+1) % n] * 10.0f);
            
            /* Additional floating point accumulation */
            f_acc += farr2[i] * 0.1f;
            
            /* Another conditional with mixed operations */
            if (arr1[i] % 7 == 0) {
                farr1[i] = sinf(f_acc) * cosf((float)arr2[i]);
            }
        }
        
        /* Cross-iteration dependency reset with variation */
        acc = (acc % 1000) + outer;
        f_acc = fmodf(f_acc, 10.0f);
    }
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays with pseudo-random data */
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
    for (int repeat = 0; repeat < 2; repeat++) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        
        /* Modify arrays slightly between repetitions */
        for (int i = 0; i < n; i++) {
            arr1[i] ^= 0x55;  /* Bitwise operation */
            farr1[i] += 0.1f * repeat;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
        checksum ^= (int)(fchecksum * 1000.0f);
    }
    
    /* Use result to prevent optimization */
    checksum = use_result(checksum);
    
    printf("Result: %d (float: %.2f)\n", checksum, fchecksum);
    return 0;
}

/* Dummy implementation to satisfy external reference */
int use_result(int x) {
    volatile int result = x;
    return result;
}
