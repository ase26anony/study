#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Function with complex loop to engage selective scheduling */
void process_data(int n, int threshold, float fthreshold) {
    /* Arrays with mixed types */
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
    int acc = 0;
    float facc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency on integers */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/float operations */
            int temp = (arr1[i] & 0xFF) + (int)(farr1[i] * 100.0f);
            
            /* Complex array indexing */
            int idx = (i * 2) % SIZE;
            arr2[i] = arr1[idx] + temp - acc % 100;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > fthreshold) {
                /* Floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i]));
                
                /* More mixed operations inside conditional */
                facc += farr2[i] * (float)(arr1[i] % 10);
                
                /* Nested condition */
                if (arr1[i] > threshold) {
                    farr2[i] *= 2.0f;
                }
            } else {
                /* Alternative path */
                farr2[i] = farr1[i] * farr1[i-1];
            }
            
            /* Bitwise operations mixed with arithmetic */
            arr1[i] = (arr1[i] ^ arr2[i-1]) + (int)farr2[i];
            
            /* Additional floating-point computation */
            if (i % 4 == 0) {
                facc = facc * 0.99f + farr1[i];
            }
        }
        
        /* Slight variation in each outer iteration */
        threshold += outer * 10;
        fthreshold += outer * 0.1f;
    }
    
    /* Use results to prevent dead code elimination */
    int checksum = acc + (int)facc + arr1[n/2] + arr2[n/2];
    use_result(checksum);
}

/* Another function with different pattern */
void process_data2(int n, int seed) {
    volatile int limit = n;  /* Volatile to prevent constant propagation */
    int data1[SIZE], data2[SIZE];
    float fdata[SIZE];
    
    srand(seed);
    for (int i = 0; i < SIZE; i++) {
        data1[i] = rand() % 256;
        data2[i] = rand() % 256;
        fdata[i] = (float)(rand() % 1000) / 1000.0f;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    for (int i = 0; i < limit; i++) {
        /* Complex addressing pattern */
        int j = (i * 3 + 7) % SIZE;
        int k = (i * 5 + 11) % SIZE;
        
        /* Multiple dependencies */
        int val = data1[j] + data2[k];
        
        /* Conditional with floating point */
        if (fdata[i] > 0.5f) {
            val *= 2;
            fsum += sinf(fdata[i]) * val;
        } else {
            val /= 2;
            fsum += cosf(fdata[i]) * val;
        }
        
        /* Update with bitwise operations */
        data1[i] = (val ^ data1[j]) & 0xFF;
        data2[i] = (val | data2[k]) & 0xFF;
        
        /* Loop-carried floating dependency */
        fdata[i] = fdata[i] * 0.9f + fsum * 0.01f;
        
        /* Integer loop-carried dependency */
        sum += val * (i % 8);
    }
    
    /* Ensure results are used */
    printf("Checksum2: %d\n", sum + (int)fsum);
}

/* Wrapper that calls both processing functions */
int main(int argc, char *argv[]) {
    /* Use command line arguments to prevent compile-time constants */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n <= 0 || n > SIZE) n = SIZE - 1;
    
    int threshold = (argc > 2) ? atoi(argv[2]) : 200;
    float fthreshold = (argc > 3) ? atof(argv[3]) : 0.3f;
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_data(n, threshold + iter * 50, fthreshold + iter * 0.1f);
        process_data2(n, rand());
    }
    
    return 0;
}

/* Dummy function definition to satisfy external reference */
int use_result(int val) {
    return val % 100;
}
