/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Target function with complex scheduling requirements */
static void __attribute__((noinline, optimize("O3")))
stress_sched(int iterations, int *result) {
    volatile int trigger = 0;
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides iteration context */
    for (k = 0; k < iterations; k++) {
        int temp1 = arr1[0];
        float ftemp1 = farr1[0];
        
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int idx1 = (i * 17) % 31;
            int idx2 = (i * 13) % 31;
            
            /* Early computation with long live range */
            int early_calc = arr1[idx1] * 3 + arr2[idx2] * 7;
            early_calc = early_calc ^ (early_calc >> 4);
            
            /* Volatile read creates scheduling barrier */
            int barrier = trigger;
            
            /* Conditional execution with side effects */
            if ((early_calc & 0xF) > 8) {
                /* Branch 1: Integer-heavy operations */
                arr1[i] = arr1[i-1] + arr2[i+1] * early_calc;
                arr2[i] = arr1[i] ^ arr2[i];
                
                /* Floating point intermixing */
                int findex = i % 16;
                farr1[findex] = farr1[findex] * 2.0f + ftemp1;
                ftemp1 = farr1[findex] * 0.5f;
            } else {
                /* Branch 2: Different arithmetic pattern */
                arr1[i] = arr2[i-1] - arr1[i+1] / (early_calc | 1);
                arr2[i] = arr1[i] | arr2[i];
                
                /* Different floating point operations */
                int findex = (i + 3) % 16;
                farr2[findex] = farr2[findex] * 3.0f - ftemp1;
                ftemp1 = farr2[findex] * 0.25f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Late use of early calculation - extends live range */
            int late_use = early_calc * 2;
            arr1[0] += late_use % 256;
            
            /* More arithmetic with register pressure */
            for (j = 0; j < 4; j++) {
                int idx3 = (i + j) % 31;
                arr2[idx3] = arr1[idx3] + arr2[idx3] * (j + 1);
            }
            
            /* Additional volatile for scheduling complexity */
            volatile int v = i;
            barrier += v;
        }
        
        /* Cross-iteration dependency */
        arr1[31] = arr1[30] + temp1;
        farr1[15] = farr1[14] + ftemp1;
        
        /* Another scheduling barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Compute result checksum */
    int sum = 0;
    for (i = 0; i < 32; i++) {
        sum += arr1[i] + arr2[i];
    }
    for (i = 0; i < 16; i++) {
        sum += (int)farr1[i] + (int)farr2[i];
    }
    *result = sum;
}

/* Helper function to create additional scheduling context */
static int __attribute__((noinline))
process_data(int seed) {
    int buffer[64];
    int i, sum = 0;
    
    for (i = 0; i < 64; i++) {
        buffer[i] = seed + i * 11;
    }
    
    /* Another complex loop pattern */
    for (i = 1; i < 63; i++) {
        if (buffer[i] % 3 == 0) {
            buffer[i] = buffer[i-1] * 2 + buffer[i+1];
        } else {
            buffer[i] = buffer[i-1] / 2 - buffer[i+1];
        }
        sum += buffer[i];
        
        /* Scheduling complexity */
        asm volatile("" ::: "memory");
    }
    
    return sum;
}

int main(void) {
    int result1, result2;
    int i;
    
    printf("Starting selective scheduling stress test...\n");
    
    /* Multiple calls to increase scheduling opportunities */
    for (i = 0; i < 3; i++) {
        stress_sched(100 + i * 50, &result1);
        result2 = process_data(result1);
        
        printf("Iteration %d: result1 = %d, result2 = %d\n", 
               i, result1, result2);
    }
    
    /* Final validation */
    stress_sched(50, &result1);
    printf("Final result: %d\n", result1);
    
    return 0;
}
