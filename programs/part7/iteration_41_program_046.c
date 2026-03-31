/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler debug output
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-verbose=5 -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 sel-sched-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

#define ARRAY_SIZE 1000

/* External function to prevent inlining and create barriers */
extern int external_barrier(int x);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, float threshold) {
    int acc = 0;  /* Loop-carried dependency */
    int temp;
    float ftemp;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Multiple memory accesses with non-linear indexing */
            int idx1 = i;
            int idx2 = i - 1;  /* Creates dependency on previous iteration */
            int idx3 = (2 * i) % n;
            
            /* Mixed integer operations with bitwise */
            temp = arr1[idx1] & 0xFF;
            temp += arr2[idx2] * 3;
            
            /* Floating-point computation */
            ftemp = farr1[idx1] * 2.5f;
            
            /* Conditional control flow - data dependent */
            if (ftemp > threshold) {
                /* Complex floating-point operation */
                farr2[idx1] = sqrtf(fabsf(ftemp));
                
                /* Integer operation dependent on float result */
                temp += (int)(farr2[idx1] * 100.0f);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = ftemp * ftemp;
                temp -= (int)(farr2[idx1]);
            }
            
            /* Loop-carried dependency update */
            acc += temp;
            
            /* Store result with dependency chain */
            arr1[idx1] = (acc & 0xFFFF) + arr2[idx3];
            
            /* Additional floating-point operation */
            farr1[idx1] = sinf(farr2[idx1] * 0.01f);
            
            /* Another conditional with mixed operations */
            if ((arr1[idx1] & 0x7) == 0) {
                arr2[i] = arr1[idx1] >> 2;
                farr1[i] = cosf(farr1[idx1]);
            }
        }
        
        /* Cross-iteration dependency */
        arr1[0] = acc % 1000;
    }
    
    /* Prevent dead code elimination */
    volatile int sink = acc;
    (void)sink;
}

/* Secondary kernel with different pattern */
void compute_kernel2(int *arr1, int *arr2, float *farr1, float *farr2,
                     int n, int seed) {
    float f_acc = 0.0f;
    int i_acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex indexing patterns */
        int idx_a = (i * 3) % n;
        int idx_b = (i + seed) % n;
        int idx_c = (i * i) % n;
        
        /* Mixed computations */
        float f_val = farr1[idx_a] + farr2[idx_b];
        int i_val = arr1[idx_c] ^ arr2[i];
        
        /* Data-dependent branching */
        if (f_val > 0.0f && i_val > 0) {
            farr1[i] = log1pf(fabsf(f_val));
            arr1[i] = i_val * 2 + (int)(farr1[i] * 10);
        } else {
            farr1[i] = expf(f_val * 0.1f) - 1.0f;
            arr1[i] = i_val / 2 - (int)(farr1[i] * 5);
        }
        
        /* Update accumulators with dependencies */
        f_acc += farr1[i];
        i_acc += arr1[i] & 0xFF;
        
        /* Periodic operation */
        if (i % 8 == 0) {
            arr2[i] = i_acc;
            farr2[i] = f_acc * 0.5f;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = ARRAY_SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > ARRAY_SIZE) {
            n = ARRAY_SIZE;
        }
    }
    
    /* Volatile to prevent constant propagation */
    volatile int volatile_n = n;
    
    /* Initialize arrays */
    int *arr1 = malloc(ARRAY_SIZE * sizeof(int));
    int *arr2 = malloc(ARRAY_SIZE * sizeof(int));
    float *farr1 = malloc(ARRAY_SIZE * sizeof(float));
    float *farr2 = malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Non-constant threshold */
    float threshold = (argc > 2) ? (float)atof(argv[2]) : 0.5f;
    
    /* Call computational kernels multiple times */
    for (int iter = 0; iter < 2; iter++) {
        compute_kernel(arr1, arr2, farr1, farr2, volatile_n, threshold);
        compute_kernel2(arr1, arr2, farr1, farr2, volatile_n, rand());
        
        /* Modify threshold slightly */
        threshold += 0.1f;
    }
    
    /* Calculate checksum to prevent elimination */
    int64_t checksum = 0;
    float fchecksum = 0.0f;
    
    for (int i = 0; i < volatile_n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
        
        /* Mix integer and float in checksum */
        checksum += (int64_t)(farr1[i] * 1000);
    }
    
    /* Print result (prevents dead code elimination) */
    printf("Checksum: %ld (float: %f)\n", checksum, fchecksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
