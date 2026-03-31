#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int threshold, float fthreshold) {
    /* Arrays with different types */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    /* Loop-carried dependency variable */
    int acc = 0;
    float facc = 0.0f;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset accumulators */
        acc = outer;
        facc = (float)outer;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            int temp = (arr1[i] & 0xFF) + (arr2[i] >> 2);
            
            /* Floating-point computation with dependency */
            facc += farr1[i] * 1.5f;
            farr2[i] = facc + (float)temp;
            
            /* Conditional control flow - data dependent */
            if (farr2[i] > fthreshold) {
                /* Complex floating-point operation */
                farr1[i] = sqrtf(fabsf(farr2[i]));
                
                /* Integer operation inside conditional */
                arr1[i] = (arr1[i] ^ arr2[i]) + (int)(farr1[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr1[i] = farr2[i] * 0.5f;
                arr1[i] = (arr1[i] & arr2[i]) | temp;
            }
            
            /* Additional conditional with integer comparison */
            if (acc > threshold && i % 4 == 0) {
                /* Memory access with non-linear indexing */
                int idx = (i * 3) % SIZE;
                arr2[idx] = arr1[i] + arr2[(i-1) % SIZE];
                
                /* More floating-point operations */
                farr2[idx] = farr1[i] * 2.0f - farr2[(i+1) % SIZE];
            }
            
            /* Cross-iteration memory dependency */
            if (i > 2) {
                arr1[i] += arr1[i-2] - arr1[i-3];
            }
        }
        
        /* Post-loop computation to prevent dead code elimination */
        for (int i = 0; i < n && i < SIZE; i++) {
            farr2[i] = farr1[i] + (float)(arr1[i] % 100);
        }
    }
    
    /* Final checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < SIZE && i < n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    /* Use results to prevent optimization */
    use_result(checksum + (int)fchecksum);
}

/* Wrapper function with volatile parameters */
void run_computation(volatile int iter_count) {
    volatile int threshold = 50000;
    volatile float fthreshold = 100.0f;
    
    /* Call processing multiple times */
    for (int run = 0; run < 2; run++) {
        process_arrays(iter_count + run, threshold + run, fthreshold + (float)run);
    }
}

int main(int argc, char *argv[]) {
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Non-constant iteration count from command line or volatile */
    volatile int n;
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        n = 500 + (rand() % 200);
    }
    
    /* Ensure n is within bounds */
    if (n < 10) n = 100;
    if (n > SIZE) n = SIZE;
    
    /* Run the computation */
    run_computation(n);
    
    printf("Computation completed with n=%d\n", n);
    return 0;
}

/* Dummy function to use results */
int use_result(int val) {
    return val % 1000;
}
