#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Function to create data dependencies and prevent optimization */
static int compute_threshold(void) {
    return rand() % 100;
}

/* External function call to prevent constant propagation */
extern int get_external_value(void);

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n <= 0) n = 500;
    
    /* Initialize with volatile to prevent compile-time simplification */
    volatile int seed = time(NULL);
    srand(seed);
    
    /* Arrays with different types to create diverse RTL patterns */
    int arr1[1000], arr2[1000], arr3[1000];
    float farr1[1000], farr2[1000];
    double darr1[1000];
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < 1000; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
        darr1[i] = (double)rand() / RAND_MAX;
    }
    
    /* Loop-carried dependency variable */
    int acc_int = 0;
    float acc_float = 0.0f;
    double acc_double = 0.0;
    
    /* Threshold from function call to prevent prediction */
    int threshold = compute_threshold();
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Core computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency - creates data hazard */
            acc_int += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            float temp_float = farr1[i] * 2.0f + (float)acc_int * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (temp_float > (threshold * 0.1f)) {
                /* Floating-point operation that may generate specific RTL */
                farr2[i] = sqrtf(fabsf(temp_float));
                
                /* Integer operation inside conditional */
                arr3[i] = (arr3[i] & 0xFF) | ((int)farr2[i] << 8);
            } else {
                /* Alternative path with different operations */
                farr2[i] = logf(fabsf(temp_float) + 1.0f);
                arr3[i] = (arr3[i] ^ 0xAA) + (int)(farr2[i] * 10.0f);
            }
            
            /* More complex data dependencies */
            acc_float += farr1[i] * farr2[i-1];
            
            /* Non-linear array indexing */
            int idx = (i * 3) % n;
            darr1[idx] = (double)arr1[i] * 0.5 + sin((double)i * 0.1);
            
            /* Bitwise operations mixed with arithmetic */
            arr1[i] = (arr1[i] << 2) | (arr2[i] & 0x3);
            arr2[i] = arr2[i] * 2 - arr1[i-1];
            
            /* Additional floating-point accumulation */
            acc_double += darr1[idx] * 0.1;
            
            /* Another conditional with different condition */
            if ((i % 7) == 0) {
                /* Complex expression with multiple operations */
                farr1[i] = (farr1[i] + farr2[i]) * (float)(arr3[i] % 100);
                acc_int -= arr3[i] / 2;
            }
        }
        
        /* Modify loop variables to prevent dead code elimination */
        n = (n * 3) % 997 + 3;
        threshold = (threshold + outer) % 100;
    }
    
    /* Checksum calculation to ensure computation isn't eliminated */
    long long checksum = 0;
    for (int i = 0; i < 1000; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + (int)farr2[i] + (int)darr1[i];
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: acc_int=%d, acc_float=%.2f, acc_double=%.2f, checksum=%lld\n",
           acc_int, acc_float, acc_double, checksum);
    
    return 0;
}
