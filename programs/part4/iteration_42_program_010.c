/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline, optimize("O3")))
stress_sched(int iterations, int *result) {
    volatile int barrier = 0;
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (k = 0; k < 32; k++) {
        arr1[k] = k * 3;
        arr2[k] = k * 7;
        if (k < 16) {
            farr1[k] = k * 1.5f;
            farr2[k] = k * 2.5f;
        }
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop - provides iteration count */
    for (i = 0; i < iterations; i++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 100; j++) {
            /* Create register pressure with many live variables */
            int idx1 = (i + j) & 31;
            int idx2 = (i * j) & 31;
            int idx3 = (i ^ j) & 15;
            int idx4 = (i - j) & 15;
            
            /* Chain of dependent integer operations */
            int t1 = arr1[idx1] * 3;
            int t2 = arr2[idx2] + t1;
            int t3 = t2 >> 2;
            int t4 = t3 ^ (t1 & 0xFF);
            
            /* Floating point operations mixed in */
            float ft1 = farr1[idx3] * 2.0f;
            float ft2 = farr2[idx4] / 1.7f;
            float ft3 = ft1 + ft2;
            
            /* Volatile read creates scheduling barrier */
            int vol_read = barrier;
            
            /* Conditional with side effects in both branches */
            if ((t4 + vol_read) & 1) {
                /* Branch 1: different arithmetic */
                t1 = t4 * 7 + arr1[(idx1 + 1) & 31];
                ft1 = ft3 * 3.14159f - farr1[(idx3 + 1) & 15];
                arr1[idx1] = t1;
                farr1[idx3] = ft1;
            } else {
                /* Branch 2: distinct operations */
                t2 = t4 / 5 - arr2[(idx2 + 1) & 31];
                ft2 = ft3 / 2.71828f + farr2[(idx4 + 1) & 15];
                arr2[idx2] = t2;
                farr2[idx4] = ft2;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of early-computed values to extend live ranges */
            int late_use1 = t1 + t2 + t3 + t4;
            float late_use2 = ft1 + ft2 + ft3;
            
            /* More operations after the barrier */
            int t5 = late_use1 ^ (late_use1 >> 16);
            float ft4 = late_use2 * late_use2;
            
            /* Accumulate results with complex addressing */
            sum += t5 + arr1[(idx1 + t5) & 31];
            fsum += ft4 + farr1[(idx3 + (int)ft4) & 15];
            
            /* Another volatile write for scheduling complexity */
            barrier = sum & 0xFF;
        }
        
        /* Cross-iteration dependencies */
        arr1[i & 31] ^= sum;
        arr2[(i + 1) & 31] += fsum;
    }
    
    *result = sum + (int)fsum;
}

/* Helper to prevent optimization */
static int __attribute__((noinline))
compute_checksum(int seed) {
    int i, result = 0;
    int buffer[64];
    
    for (i = 0; i < 64; i++) {
        buffer[i] = (seed + i * 1103515245) & 0x7FFFFFFF;
    }
    
    for (i = 0; i < 1000; i++) {
        int idx = i & 63;
        buffer[idx] = (buffer[idx] * 1664525 + 1013904223) & 0x7FFFFFFF;
        result ^= buffer[idx];
    }
    
    return result;
}

int main(void) {
    int i;
    int final_result = 0;
    int checksum = 0;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Call target function multiple times with different parameters */
    for (i = 0; i < 10; i++) {
        int iter_count = 50 + (i * 3);
        int partial_result;
        
        stress_sched(iter_count, &partial_result);
        final_result += partial_result;
        
        /* Compute checksum to ensure code executes */
        checksum += compute_checksum(partial_result);
        
        printf("Iteration %d: result = %d, checksum = %d\n", 
               i, partial_result, checksum);
    }
    
    printf("Final result: %d\n", final_result);
    printf("Final checksum: %d\n", checksum);
    
    return (final_result + checksum) != 0 ? 0 : 1;
}
