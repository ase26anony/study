#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int get_value(void);

/* Function with complex loop to engage selective scheduling */
void process_arrays(int n, int threshold, float fthreshold) {
    /* Arrays with different types */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    /* Loop-carried dependency variable */
    int accumulator = 0;
    
    /* Use volatile to prevent compile-time optimization */
    volatile int loop_limit = n;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Complex inner loop with mixed operations */
        for (int i = 1; i < loop_limit; i++) {
            /* Loop-carried dependency */
            accumulator += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] >> 2);
            
            /* Floating-point computation */
            float fval = farr1[i] * 2.0f + sinf(farr1[i-1]);
            
            /* Conditional with data-dependent branch */
            if (fval > fthreshold) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(fval)) + cosf(farr1[i]);
                
                /* Mixed-type conversion */
                arr1[i] = temp + (int)(farr2[i] * 10.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = fval * 0.5f;
                arr1[i] = temp - (int)(farr2[i] * 5.0f);
            }
            
            /* Additional conditional with unpredictable branch */
            if (accumulator > threshold) {
                /* More mixed operations */
                farr1[i] = logf(fabsf(farr2[i]) + 1.0f);
                accumulator = accumulator / 2;
            }
            
            /* Cross-iteration memory access pattern */
            arr2[i] = arr1[i] + arr2[i-1] + (int)(sinf((float)i) * 100.0f);
        }
        
        /* Modify threshold to vary branch behavior */
        threshold += get_value();
    }
    
    /* Prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < loop_limit; i++) {
        checksum += arr1[i] + (int)farr2[i];
    }
    printf("Checksum: %d\n", checksum);
}

/* External function implementation */
int get_value(void) {
    return rand() % 10;
}

int main(int argc, char *argv[]) {
    /* Seed RNG for unpredictable values */
    srand(time(NULL));
    
    /* Use command-line argument for non-constant loop bound */
    int n = 500; /* Default */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = 500;
    }
    
    /* Volatile variables to prevent constant propagation */
    volatile int threshold = rand() % 1000 + 500;
    volatile float fthreshold = (float)rand() / RAND_MAX;
    
    /* Call the processing function multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n + iter * 10, threshold, fthreshold);
    }
    
    return 0;
}
