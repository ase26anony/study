#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Function to prevent dead code elimination */
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}

/* Complex loop with mixed operations and dependencies */
void process_data(int n, float threshold, int *arr1, int *arr2, 
                  float *farr1, float *farr2, int *results) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Reset accumulators */
        acc = outer;
        f_acc = (float)outer;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n - 1; ++i) {
            /* Loop-carried dependency on acc */
            int temp = arr1[i] * arr2[i];
            acc += temp;
            
            /* Mixed integer/float operations */
            float f_val = farr1[i] + (float)acc * 0.01f;
            f_acc = f_acc * 0.99f + f_val;
            
            /* Conditional control flow with data-dependent condition */
            if (f_val > threshold && (acc & 0x3F) != 0) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(f_val)) + 
                          sinf((float)i * 0.01f) * 
                          cosf((float)acc * 0.001f);
                
                /* Integer operation dependent on float result */
                arr1[i+1] = (arr1[i] & 0xFF) + 
                           (int)(farr2[i] * 100.0f) + 
                           arr2[i-1];
            } else {
                /* Alternative computation path */
                farr2[i] = f_val * 0.5f;
                arr1[i+1] = (arr1[i] >> 2) | (arr2[i] << 3);
            }
            
            /* More mixed operations */
            results[i] = (results[i-1] ^ temp) + 
                        ((int)farr2[i] & 0xFFFF);
            
            /* Additional floating-point computation */
            farr1[i+1] = farr1[i] * 0.9f + 
                        farr2[i] * 0.1f + 
                        (float)(i % 10);
        }
        
        /* Cross-iteration dependency */
        arr1[0] = acc % 1000;
        farr1[0] = f_acc;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    
    /* Ensure n is reasonable and odd to prevent optimization */
    if (n < 100) n = 100;
    if (n > 1000) n = 1000;
    n |= 1;  /* Make odd to break patterns */
    
    /* Volatile variable to prevent optimization */
    volatile int v_n = n;
    
    /* Initialize arrays */
    int *arr1 = malloc(v_n * sizeof(int));
    int *arr2 = malloc(v_n * sizeof(int));
    float *farr1 = malloc(v_n * sizeof(float));
    float *farr2 = malloc(v_n * sizeof(float));
    int *results = malloc(v_n * sizeof(int));
    
    if (!arr1 || !arr2 || !farr1 || !farr2 || !results) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < v_n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / (float)RAND_MAX * 10.0f;
        farr2[i] = 0.0f;
        results[i] = 0;
    }
    
    /* Data-dependent threshold */
    float threshold = (float)(rand() % 100) / 100.0f + 0.3f;
    
    /* Process the data multiple times */
    for (int repeat = 0; repeat < 2; ++repeat) {
        process_data(v_n, threshold, arr1, arr2, farr1, farr2, results);
        
        /* Modify threshold slightly each iteration */
        threshold += 0.05f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    double f_checksum = 0.0;
    
    for (int i = 0; i < v_n; ++i) {
        checksum += arr1[i] + arr2[i] + results[i];
        f_checksum += farr1[i] + farr2[i];
        
        /* Prevent optimization */
        escape(&arr1[i]);
        escape(&arr2[i]);
        escape(&farr1[i]);
        escape(&farr2[i]);
        escape(&results[i]);
    }
    
    /* Print results to ensure computation isn't eliminated */
    printf("Checksums: %lld, %.6f\n", checksum, f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(results);
    
    return 0;
}
