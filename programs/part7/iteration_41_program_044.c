#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining */
extern int external_helper(int x);

/* Function with complex loop to engage selective scheduling */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            float temp = (float)acc * 0.01f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f) {
                /* Floating-point operation */
                farr2[i] = sqrtf(farr1[i] + temp);
                
                /* Bitwise operation combined with float conversion */
                arr1[i] = (arr1[i] & 0xFF) | ((int)(farr2[i] * 100.0f) << 8);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1] + temp;
                arr1[i] = (arr1[i] ^ 0x55) + (int)(farr2[i]);
            }
            
            /* More complex memory access pattern */
            int idx = (i * 2) % n;
            arr2[i] = arr1[idx] + arr2[(i + 1) % n] - acc;
            
            /* Additional floating-point computation */
            f_acc += farr2[i] * sinf((float)i * 0.01f);
            
            /* Another conditional with mixed operations */
            if ((arr1[i] & 0xF) > 8) {
                farr1[i] = cosf(f_acc) * 0.5f + 0.5f;
            }
        }
        
        /* Small perturbation between outer iterations */
        acc = (acc >> 1) | (acc << 31);  /* Rotate bits */
        f_acc = f_acc * 0.9f;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink = acc + (int)f_acc;
    (void)sink;
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays with pseudo-random data */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Call processing function multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < n; i += 7) {
            arr1[i] += iter;
            farr1[i] += iter * 0.1f;
        }
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)(farr1[i] * 100) + (int)(farr2[i] * 100);
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* External function definition to prevent optimization */
int external_helper(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}
