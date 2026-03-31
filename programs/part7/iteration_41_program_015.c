/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler debug output
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
void process_arrays(int n, int *arr1, int *arr2, float *farr1, float *farr2, 
                    int *dep_arr, float threshold) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Reset accumulators */
        acc = external_func(outer);  /* Prevent constant propagation */
        f_acc = (float)acc * 0.01f;
        
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency on integer accumulator */
            int prev_acc = acc;
            
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = (i * 2) % n;
            int idx3 = (i + 1) % n;
            
            /* Integer arithmetic with bitwise operations */
            int temp1 = arr1[idx1] & 0xFF;
            int temp2 = arr2[idx2] | 0x7F;
            int temp3 = dep_arr[i-1] ^ 0x55;  /* Dependency on previous iteration */
            
            /* Mixed integer operations */
            acc += temp1 * temp2 - temp3;
            
            /* Floating-point operations */
            float f_val1 = farr1[idx1];
            float f_val2 = farr2[idx3];
            
            /* Conditional control flow with data-dependent condition */
            if (f_val1 > threshold && (acc % 7) != 0) {
                /* Complex floating-point computation */
                f_val2 = sqrtf(fabsf(f_val1)) + f_acc;
                farr2[idx3] = f_val2;
                
                /* More integer operations inside conditional */
                arr1[idx1] = (arr1[idx1] << 2) | (temp2 & 0x3);
            } else {
                /* Alternative computation path */
                f_val2 = f_val1 * f_val1 - f_acc;
                farr2[idx3] = f_val2;
                
                arr1[idx1] = (arr1[idx1] >> 1) + temp3;
            }
            
            /* Update floating accumulator with mixed operations */
            f_acc = f_acc * 0.95f + f_val2 * 0.05f;
            
            /* Store dependency for next iteration */
            dep_arr[i] = acc + (int)f_acc;
            
            /* Additional conditional with unpredictable branch */
            if ((arr1[idx1] + arr2[idx2]) % 13 == 0) {
                /* More operations in nested conditional */
                float temp_f = sinf(f_val1) * cosf(f_val2);
                farr1[idx1] = temp_f * 100.0f;
                
                /* Bit manipulation */
                arr2[idx2] = (arr2[idx2] ^ prev_acc) & 0xFFFF;
            }
            
            /* Final mixed-type computation */
            float final_f = (float)acc * 0.001f + f_acc;
            int final_int = (int)final_f + arr1[idx1] - arr2[idx2];
            
            /* Store result with dependency chain */
            dep_arr[i] = final_int ^ dep_arr[i-1];
        }
    }
}

/* Checksum function to prevent dead code elimination */
int compute_checksum(int n, int *arr1, int *arr2, float *farr1, float *farr2, int *dep_arr) {
    int checksum = 0;
    float f_checksum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        checksum ^= arr1[i];
        checksum += arr2[i] * 3;
        checksum ^= (int)(farr1[i] * 1000.0f);
        checksum += (int)(farr2[i] * 100.0f);
        checksum ^= dep_arr[i];
        
        f_checksum += farr1[i] * 0.01f + farr2[i] * 0.02f;
    }
    
    return checksum + (int)f_checksum;
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time constant propagation */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > ARRAY_SIZE) n = ARRAY_SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays */
    int *arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *dep_arr = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    /* Seed RNG with time for unpredictable values */
    srand(time(NULL));
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        dep_arr[i] = rand() % 256;
    }
    
    /* Volatile threshold to prevent constant folding */
    volatile float threshold = 0.25f + (argc * 0.01f);
    
    /* Process arrays - this should trigger selective scheduling */
    process_arrays(n, arr1, arr2, farr1, farr2, dep_arr, threshold);
    
    /* Compute and print checksum to ensure computation isn't eliminated */
    int checksum = compute_checksum(n, arr1, arr2, farr1, farr2, dep_arr);
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(dep_arr);
    
    return 0;
}

/* Dummy external function definition */
int external_func(int x) {
    return (x * 37) ^ 0xDEADBEEF;
}
