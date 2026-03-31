#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int external_func(int);

void process_loop(int n, int threshold) {
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    volatile int limit = n;  /* Prevent constant propagation */
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    int loop_carried_acc = 0;
    float fp_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Core computational loop with complex dependencies */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried dependency */
            loop_carried_acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] >> 3);
            temp = temp ^ (i * 7);
            
            /* Floating-point computation */
            fp_acc += farr1[i] * 2.5f;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f && (temp % 3) == 0) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i] * 2.0f - 1.0f));
                
                /* Integer operation inside conditional */
                arr1[i] = (int)(farr2[i] * 100) + temp;
            } else {
                /* Alternative path with different operations */
                farr2[i] = powf(farr1[i], 1.5f);
                arr1[i] = (arr1[i] * 3) / 2;
            }
            
            /* Cross-type dependency */
            arr2[i] = (int)(farr2[i] * 10) + loop_carried_acc % 100;
            
            /* Additional floating-point with conversion */
            float ftemp = (float)arr1[i] / (float)(arr2[i] + 1);
            fp_acc = fp_acc * 0.99f + ftemp;
            
            /* Memory access with non-linear indexing */
            if (i * 2 < SIZE) {
                arr1[i] += arr2[i * 2] / 2;
            }
        }
        
        /* Slight variation in each outer iteration */
        limit = limit - (outer * 10);
        if (limit < 100) limit = 100;
    }
    
    /* Prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr2[i];
    }
    checksum += (int)fp_acc + loop_carried_acc;
    
    /* Use result to prevent optimization */
    if (external_func(checksum) > 1000) {
        printf("Result: %d\n", checksum);
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    /* Non-constant loop bound from command line */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100 || n > 800) n = 500;
    }
    
    /* Data-dependent threshold */
    int threshold = rand() % 100 + 50;
    
    /* Call the processing function multiple times */
    for (int repeat = 0; repeat < 2; repeat++) {
        process_loop(n + repeat * 50, threshold + repeat * 10);
    }
    
    return 0;
}

/* Dummy external function definition */
int external_func(int x) {
    return x % 1000;
}
