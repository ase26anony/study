/* Test case to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int iterations) 
{
    volatile int seed = 12345;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    int i, j;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop for sufficient iterations */
    for (j = 0; j < iterations; j++) {
        int idx = j & 31;
        int temp1, temp2;
        float ftemp1, ftemp2;
        
        /* Create register pressure with mixed calculations */
        temp1 = arr1[idx] * 3 + arr2[(idx + 1) & 31] * 7;
        temp2 = arr1[(idx + 2) & 31] - arr2[(idx + 3) & 31];
        ftemp1 = farr1[idx] * 1.7f + farr2[(idx + 1) & 31] * 3.2f;
        ftemp2 = farr1[(idx + 2) & 31] / 2.0f - farr2[(idx + 3) & 31];
        
        /* Volatile read creates scheduling barrier */
        int barrier = seed;
        
        /* Complex conditional with side effects */
        if ((temp1 * temp2 + barrier) > 1000) {
            /* Branch 1: Integer-heavy operations */
            arr1[idx] = temp1 * 2 + barrier;
            arr2[(idx + 1) & 31] = temp2 / 3 - barrier;
            
            /* Floating-point chain with dependencies */
            ftemp1 = ftemp1 * 2.0f + (float)barrier;
            farr1[idx] = ftemp1 * 1.1f;
            ftemp2 = ftemp2 / 1.5f - (float)(barrier & 0xFF);
            farr2[(idx + 1) & 31] = ftemp2 * 0.9f;
        } else {
            /* Branch 2: Different operations */
            arr1[(idx + 4) & 31] = temp1 + temp2 * barrier;
            arr2[(idx + 5) & 31] = temp1 - temp2 / (barrier + 1);
            
            /* Alternative FP operations */
            ftemp1 = ftemp1 / 3.0f * (float)(barrier % 100);
            farr1[(idx + 4) & 31] = ftemp1 + 1.0f;
            ftemp2 = ftemp2 * 4.0f - (float)(barrier >> 2);
            farr2[(idx + 5) & 31] = ftemp2 / 2.0f;
        }
        
        /* Inline assembly as scheduling boundary */
        asm volatile("" ::: "memory");
        
        /* Late use of early-computed values to extend live ranges */
        int late_use1 = temp1 * 3 + arr1[(idx + 6) & 31];
        int late_use2 = temp2 / 2 + arr2[(idx + 7) & 31];
        float late_fuse1 = ftemp1 * 1.3f + farr1[(idx + 6) & 31];
        float late_fuse2 = ftemp2 / 1.7f - farr2[(idx + 7) & 31];
        
        /* Store results with complex indexing */
        arr1[(idx + 8) & 31] = late_use1 + late_use2;
        arr2[(idx + 9) & 31] = late_use1 - late_use2;
        farr1[(idx + 8) & 31] = late_fuse1 + late_fuse2;
        farr2[(idx + 9) & 31] = late_fuse1 - late_fuse2;
        
        /* Another volatile write barrier */
        seed = barrier + 1;
    }
}

/* Main driver */
int main() {
    int i;
    int checksum = 0;
    
    /* Call target function multiple times */
    for (i = 0; i < 100; i++) {
        stress_sched(1000);
        checksum += i * 3;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
