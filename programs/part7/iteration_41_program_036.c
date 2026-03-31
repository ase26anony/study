/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler verbose debugging output
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-verbose=5 -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 sel-sched-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <stdint.h>

/* External function to prevent inlining and create barriers */
extern int external_barrier(int x);

/* Complex loop with mixed operations and dependencies */
__attribute__((noinline))
static int complex_loop(int *arr1, int *arr2, float *farr1, float *farr2, 
                        volatile int n, float threshold) {
    int loop_carried_acc = 0;
    float fp_acc = 0.0f;
    int bitmask_acc = 0;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            int temp = arr1[i] * arr2[i-1];
            loop_carried_acc += temp;
            
            /* Floating-point computation with data-dependent indexing */
            float fp_val = farr1[i] * 1.5f + farr1[i-1] * 0.5f;
            fp_acc += fp_val;
            
            /* Complex array indexing with non-linear pattern */
            int idx = (i * 7) % n;
            int idx2 = (i * 13) % n;
            
            /* Mixed integer/float operations */
            float mixed = (float)(arr1[idx] & 0xFF) + fp_val;
            
            /* Conditional control flow - data dependent */
            if (farr1[i] > threshold && (arr1[i] % 3) != 0) {
                /* Memory store with computation */
                farr2[i] = sqrtf(fabsf(mixed)) + fp_acc;
                
                /* Bitwise operations */
                arr2[i] = (arr2[i] ^ arr1[idx2]) | (arr1[i] << 2);
                
                /* Additional floating-point op */
                fp_val = sinf(farr2[i] * 0.01f);
            } else {
                /* Alternative path with different operations */
                farr2[i] = logf(fabsf(mixed) + 1.0f);
                arr2[i] = (arr2[i] & 0xFFFF) + (int)(fp_val * 100.0f);
            }
            
            /* More mixed operations */
            int bit_op = (arr1[i] >> 3) & 0xF;
            bitmask_acc ^= bit_op;
            
            /* Floating-point accumulation with conditional */
            if (bit_op > 5) {
                fp_acc += cosf((float)i * 0.1f);
            }
            
            /* Cross-iteration store with dependency */
            arr1[i] = arr1[i-1] + (int)(farr2[i] * 10.0f) + bitmask_acc;
            
            /* Prevent over-optimization barrier */
            if (external_barrier(i) > 1000) break;
        }
        
        /* Shuffle data between outer iterations */
        for (int i = 0; i < n-1; i++) {
            arr1[i] ^= arr2[i+1];
            farr1[i] = farr1[i] * 0.9f + farr2[i+1] * 0.1f;
        }
    }
    
    /* Combine accumulators to prevent elimination */
    return loop_carried_acc + (int)fp_acc + bitmask_acc;
}

/* Simple external barrier function */
int external_barrier(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter & 0xFF;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    volatile int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 1000) n = 1000;
    }
    
    /* Initialize with random data */
    srand(time(NULL));
    
    /* Allocate arrays with mixed types */
    int *arr1 = malloc(n * sizeof(int));
    int *arr2 = malloc(n * sizeof(int));
    float *farr1 = malloc(n * sizeof(float));
    float *farr2 = malloc(n * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
    }
    
    /* Data-dependent threshold */
    float threshold = (float)(rand() % 100) / 100.0f;
    
    /* Execute the complex loop */
    int result = complex_loop(arr1, arr2, farr1, farr2, n, threshold);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    float fp_checksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] ^ arr2[i];
        fp_checksum += farr1[i] + farr2[i];
    }
    
    /* Use results to prevent optimization */
    printf("Result: %d, Checksum: %d, FP Checksum: %f\n", 
           result, checksum, fp_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
