#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* External function to prevent inlining and create more complex RTL */
extern int external_helper(int x);

/* Function with complex loop that should trigger selective scheduling */
void process_data(int n, float threshold, int* arr1, int* arr2, 
                  float* farr1, float* farr2, int* results) {
    int i;
    volatile int limit = n; /* Prevent constant propagation */
    int acc = 0;
    int bitmask = 0xFF;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        acc = 0; /* Reset accumulator each outer iteration */
        
        /* Main computational loop with mixed operations */
        for (i = 1; i < limit; i++) {
            /* Loop-carried dependency */
            int prev = results[i-1];
            
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = (i * 2) % n;
            int idx3 = (i + prev) % n;
            
            /* Integer arithmetic with data dependencies */
            int temp1 = arr1[idx1] * arr2[idx2];
            int temp2 = arr1[idx3] + external_helper(prev);
            
            /* Update accumulator with loop-carried dependency */
            acc += temp1 - temp2;
            results[i] = acc;
            
            /* Floating-point operations */
            float fval = farr1[i] * 2.0f;
            farr2[i] = fval;
            
            /* Conditional control flow inside loop */
            if (fval > threshold && (acc & 1)) {
                /* Complex conditional block */
                farr2[i] = sqrtf(fabsf(fval));
                
                /* Bitwise operations mixed with FP */
                arr1[i] = (arr1[i] & bitmask) | ((int)farr2[i] << 8);
                
                /* Additional dependency chain */
                arr2[i] = arr2[i-1] + (arr1[i] % 256);
            } else {
                /* Alternative path */
                farr2[i] = fval * 0.5f;
                arr1[i] = (arr1[i] ^ bitmask) - (int)fval;
            }
            
            /* Mixed-type computation */
            float mixed = (float)acc * 0.01f + farr2[i];
            
            /* Another conditional with data-dependent condition */
            if (mixed > 100.0f || acc < 0) {
                farr1[i] = mixed * 0.8f;
                arr2[i] = (arr2[i] << 2) | (arr2[i] >> 6);
            }
            
            /* Prevent loop unrolling from simplifying too much */
            if (i % 7 == 0) {
                bitmask = (bitmask * 13 + 1) & 0xFF;
            }
        }
        
        /* Cross-iteration dependency for outer loop */
        arr1[0] += acc;
    }
}

/* External function definition */
int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 1000) n = 1000;
    }
    
    /* Initialize arrays */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr1 = (float*)malloc(n * sizeof(float));
    float* farr2 = (float*)malloc(n * sizeof(float));
    int* results = (int*)malloc(n * sizeof(int));
    
    if (!arr1 || !arr2 || !farr1 || !farr2 || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 10.0f;
        farr2[i] = 0.0f;
        results[i] = 0;
    }
    
    /* Set initial value for loop-carried dependency */
    results[0] = rand() % 100;
    
    /* Volatile threshold to prevent constant propagation */
    volatile float threshold = 2.5f;
    
    /* Process the data - this is where selective scheduling should activate */
    process_data(n, threshold, arr1, arr2, farr1, farr2, results);
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i] + results[i];
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(results);
    
    return 0;
}
