/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler verbose debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-verbose=5 -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 sel-sched-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ARRAY_SIZE 1000

/* External function to prevent optimization */
extern int external_func(int);

/* Complex loop with mixed operations and dependencies */
void compute_loop(int *arr1, int *arr2, float *farr1, float *farr2, 
                  volatile int limit, int threshold) {
    int acc = 0;  /* Loop-carried dependency */
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < limit; i++) {
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = i - 1;  /* Creates dependency on previous iteration */
            int idx3 = (i * 2) % ARRAY_SIZE;
            
            /* Loop-carried integer dependency */
            acc += arr1[idx1] * arr2[idx2] + external_func(i % 10);
            
            /* Floating-point computation with data-dependent condition */
            float temp = farr1[idx1] * 1.5f - farr2[idx2];
            f_acc += temp;
            
            /* Conditional control flow inside loop */
            if (farr1[idx1] > 0.5f && (acc % 100) > threshold) {
                /* Complex floating-point operation */
                farr2[idx1] = sqrtf(fabsf(farr1[idx1])) + 
                             sinf(farr2[idx2] * 0.01f);
                
                /* Mixed integer/float operations */
                arr1[idx3] = (arr1[idx3] & 0xFF) + (int)(farr2[idx1] * 10.0f);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = farr1[idx1] * farr1[idx1] - farr2[idx2];
                arr1[idx3] = (arr1[idx3] | 0x7F) - (int)(farr2[idx1] / 2.0f);
            }
            
            /* Additional integer computation with bitwise ops */
            arr2[idx1] = (arr1[idx1] ^ arr2[idx2]) + 
                        ((arr1[idx3] << 2) & 0x3FF);
            
            /* Another floating-point operation */
            farr1[idx1] = farr2[idx1] * 0.9f + farr1[idx2] * 0.1f;
            
            /* Complex condition for another branch */
            if ((arr1[idx1] + arr2[idx2]) > (threshold * 2)) {
                /* More mixed operations */
                f_acc -= cosf(farr1[idx1]) * 0.5f;
                arr1[idx1] = (arr1[idx1] * 3) / 2;
            }
        }
        
        /* Small variation in each outer iteration */
        threshold = (threshold + 1) % 50;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int result = acc + (int)f_acc;
    (void)result;  /* Prevent unused variable warning */
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, float *farr1, float *farr2) {
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
}

/* Calculate checksum to ensure computation isn't optimized away */
long long calculate_checksum(int *arr1, int *arr2, float *farr1, float *farr2) {
    long long checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += arr1[i] + arr2[i] + (long long)(farr1[i] * 1000) + 
                   (long long)(farr2[i] * 1000);
    }
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Seed RNG with time for variability */
    srand(time(NULL));
    
    /* Allocate arrays */
    int arr1[ARRAY_SIZE], arr2[ARRAY_SIZE];
    float farr1[ARRAY_SIZE], farr2[ARRAY_SIZE];
    
    /* Initialize with random data */
    init_arrays(arr1, arr2, farr1, farr2);
    
    /* Use command-line argument or volatile for loop limit */
    volatile int limit;
    if (argc > 1) {
        limit = atoi(argv[1]);
        if (limit <= 0 || limit > ARRAY_SIZE - 1)
            limit = ARRAY_SIZE - 1;
    } else {
        limit = 500 + (rand() % 400);  /* Non-constant loop bound */
    }
    
    /* Data-dependent threshold */
    int threshold = rand() % 100;
    
    /* Perform computation */
    compute_loop(arr1, arr2, farr1, farr2, limit, threshold);
    
    /* Calculate and print checksum */
    long long checksum = calculate_checksum(arr1, arr2, farr1, farr2);
    printf("Checksum: %lld\n", checksum);
    
    return 0;
}

/* Dummy external function definition */
int external_func(int x) {
    return (x * 37) % 101;
}
