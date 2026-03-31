#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset accumulators */
        acc = outer;
        f_acc = (float)outer;
        
        /* Main complex loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            int prev = arr1[i-1];
            acc += prev * arr2[i];
            
            /* Mixed integer/float operations */
            float temp = (float)acc * 0.01f;
            f_acc = f_acc + temp;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > f_acc) {
                /* Complex floating point operation */
                farr2[i] = sqrtf(fabsf(farr1[i] - f_acc));
                
                /* Integer operation dependent on float result */
                arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative path with different operations */
                farr2[i] = powf(farr1[i], 1.5f);
                arr1[i] = (arr1[i] >> 2) | (arr2[i] << 4);
            }
            
            /* More mixed operations */
            arr2[i] = (arr2[i] ^ prev) + (int)farr2[i];
            
            /* Additional floating point operation */
            farr1[i] = sinf(farr1[i]) * cosf(f_acc);
            
            /* Another conditional with bitwise operations */
            if ((arr1[i] & 0x7) == 0) {
                farr2[i] = farr2[i] * 2.0f;
                arr2[i] = arr2[i] + (i & 0xF);
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
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
        farr1[i] = (float)rand() / RAND_MAX * 10.0f;
        farr2[i] = 0.0f;
    }
    
    /* Process arrays multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        
        /* Shuffle data slightly between iterations */
        for (int i = 1; i < n; i++) {
            arr1[i] = (arr1[i] + arr2[i-1]) & 0x3FF;
            farr1[i] = fmodf(farr1[i] + farr2[i-1], 5.0f);
        }
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum ^= arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, f_checksum);
    return 0;
}

/* External function definition */
int use_result(int val) {
    return val % 100;
}
