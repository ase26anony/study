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
    
    /* Loop-carried accumulator */
    int acc = 0;
    float facc = 0.0f;
    
    /* Volatile variable to prevent constant propagation */
    volatile int vol_n = n;
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 256;
        arr2[i] = rand() % 256;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset accumulators */
        acc = outer;
        facc = (float)outer;
        
        /* Main complex loop with dependencies */
        for (int i = 1; i < vol_n; i++) {
            /* Loop-carried dependency on integer accumulator */
            acc += arr1[i] * arr2[i-1];
            
            /* Complex indexing with multiple dependencies */
            int idx = (i * 2) % SIZE;
            int idx2 = (i * 3) % SIZE;
            
            /* Mixed integer operations with bitwise */
            arr1[i] = (arr1[i] & 0xFF) + (arr2[idx] >> 2);
            
            /* Floating-point computation with dependency */
            farr2[i] = farr1[i] * facc + farr2[i-1] * 0.5f;
            
            /* Conditional control flow - data dependent */
            if (farr1[i] > fthreshold) {
                /* More complex FP operation inside condition */
                farr2[i] = sqrtf(fabsf(farr2[i]));
                
                /* Integer operation inside FP condition */
                arr2[i] = arr2[i] ^ (arr1[i] & 0xF);
            } else {
                /* Alternative computation path */
                farr2[i] = powf(farr2[i], 1.5f);
                arr2[i] = arr2[i] | 0x1;
            }
            
            /* Another conditional with integer comparison */
            if (acc > threshold && (i % 7) == 0) {
                /* Cross-type conversion */
                facc += (float)acc * 0.01f;
                
                /* Memory access with stride */
                arr1[(i * 5) % SIZE] = arr2[(i * 3) % SIZE] + 
                                       (int)(farr2[i] * 10.0f);
            }
            
            /* Update floating accumulator with mixed operations */
            facc = facc * 0.99f + farr2[i] * 0.1f;
            
            /* Additional dependency chain */
            arr2[i] = arr2[i] + (arr1[i-1] & 0x7F);
        }
        
        /* Cross-iteration dependency between outer loops */
        if (outer > 0) {
            for (int i = 0; i < 10; i++) {
                int idx = (outer * i) % SIZE;
                arr1[idx] += arr2[idx] * outer;
                farr2[idx] += (float)arr1[idx] * 0.01f;
            }
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)farr2[i];
    }
    
    /* Use result to prevent optimization */
    if (use_result(checksum)) {
        printf("Checksum: %d\n", checksum);
    }
}

/* Main function with command-line arguments */
int main(int argc, char *argv[]) {
    /* Seed RNG */
    srand(time(NULL));
    
    /* Use command-line argument for variable loop bound */
    int n = SIZE - 100;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE - 100;
    }
    
    /* Volatile thresholds to prevent constant folding */
    volatile int threshold = 10000;
    volatile float fthreshold = 0.25f;
    
    /* Call the processing function multiple times */
    for (int repeat = 0; repeat < 2; repeat++) {
        process_arrays(n + repeat * 50, 
                      threshold + repeat * 1000,
                      fthreshold + repeat * 0.1f);
    }
    
    return 0;
}

/* External function definition */
int use_result(int val) {
    volatile int result = val;
    return result != 0;
}
