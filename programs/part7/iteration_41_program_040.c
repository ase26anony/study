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
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency: depends on previous iteration */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* Floating-point computation */
            float fval = farr1[i] * 2.5f + sinf(farr1[i-1]);
            
            /* Conditional control flow - data dependent */
            if (fval > fthreshold) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(fval)) + cosf(farr1[i]);
                
                /* Integer operation inside conditional */
                arr1[i] = temp + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative path */
                farr2[i] = fval * 0.5f;
                arr1[i] = temp - (int)(farr2[i] * 50.0f);
            }
            
            /* Another conditional with integer comparison */
            if (acc > threshold) {
                /* Reset with non-trivial operation */
                acc = (acc & 0x7FFF) | ((arr1[i] & 0xFF) << 15);
                
                /* Memory access with non-linear indexing */
                int idx = (i * 3) % SIZE;
                arr2[idx] = arr1[i] ^ arr2[(i-1) % SIZE];
            }
            
            /* Additional floating-point operation */
            farr1[i] = farr2[i] * 0.9f + farr1[i-1] * 0.1f;
            
            /* Mixed-type operation crossing integer/float */
            if (i % 4 == 0) {
                farr2[i] += (float)(arr1[i] % 100);
            }
        }
        
        /* Slight variation in each outer iteration */
        threshold += outer * 100;
        fthreshold += outer * 0.1f;
    }
    
    /* Prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + (int)farr2[i];
    }
    
    /* Use result to prevent optimization */
    if (use_result(checksum)) {
        printf("Checksum: %d\n", checksum);
    }
}

/* Second function with different pattern to increase scheduling complexity */
void process_data_alternate(int n, int seed) {
    volatile int limit = n;  /* Volatile to prevent constant propagation */
    float data[SIZE];
    int results[SIZE];
    
    srand(seed);
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)rand() / RAND_MAX;
        results[i] = 0;
    }
    
    float accum = 0.0f;
    for (int i = 0; i < limit; i++) {
        /* Complex dependency chain */
        float x = data[i];
        float y = data[(i + 1) % SIZE];
        
        /* Branch with floating-point comparison */
        if (x > 0.5f && y < 0.8f) {
            accum = accum * 0.95f + x * y;
            results[i] = (int)(accum * 1000.0f);
        } else {
            accum = accum * 0.8f - x + y;
            results[i] = (int)(accum * 500.0f);
        }
        
        /* Trigonometric operations */
        if (i % 3 == 0) {
            data[i] = sinf(accum) + cosf(x * 3.14159f);
        }
        
        /* Integer operation with bit manipulation */
        results[i] = (results[i] & 0xFFFF) | ((i & 0xFF) << 16);
    }
    
    /* Use result */
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        sum += results[i];
    }
    use_result(sum);
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n < 10) n = 500;
    if (n > SIZE) n = SIZE;
    
    /* Volatile variables to prevent optimization */
    volatile int threshold = 10000;
    volatile float fthreshold = 0.25f;
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Call both processing functions */
    process_data(n, threshold, fthreshold);
    process_data_alternate(n / 2, rand());
    
    return 0;
}

/* Dummy function definition */
int use_result(int x) {
    return x != 0;
}
