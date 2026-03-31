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
        arr2[i] = i * 5;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < iterations; j++) {
        int temp_int = seed;
        float temp_float = seed * 0.5f;
        
        /* Complex inner loop with high ILP and register pressure */
        for (i = 0; i < 32; i++) {
            /* Chain of dependent integer operations */
            int idx1 = (temp_int + i) & 31;
            int idx2 = (temp_int - i * 7) & 31;
            int idx3 = (temp_int ^ (i * 11)) & 31;
            
            /* Multiple arithmetic operations creating dependencies */
            int val1 = arr1[idx1] * 3 + arr2[idx2];
            int val2 = arr1[idx2] / 2 - arr1[idx3];
            int val3 = val1 ^ val2;
            int val4 = val3 * 7 + (val1 & 0xFF);
            
            /* Floating point operations mixed in */
            float fval1 = farr1[idx1] * 1.7f + farr2[idx2];
            float fval2 = farr1[idx2] / 1.3f - farr1[idx3];
            float fval3 = fval1 * fval2 + temp_float;
            
            /* Conditional with side effects in both branches */
            if ((val4 & 0x3) == 0) {
                /* Branch 1: different operations */
                arr1[idx1] = val4 + (int)(fval3 * 10.0f);
                farr1[idx2] = fval3 * 0.9f + fval1;
                temp_int += val3 * 3;
            } else {
                /* Branch 2: alternative operations */
                arr2[idx3] = val4 - (int)(fval3 * 5.0f);
                farr2[idx1] = fval3 * 1.1f - fval2;
                temp_int -= val2 * 2;
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* More operations extending live ranges */
            int late_use1 = arr1[(idx1 + 1) & 31] + temp_int;
            int late_use2 = arr2[(idx2 + 2) & 31] - temp_int;
            float late_use3 = farr1[(idx3 + 3) & 31] * temp_float;
            
            /* Final computations using values from much earlier */
            arr1[i] = (val1 + late_use1) & 0xFFFF;
            arr2[i] = (val2 - late_use2) & 0xFFFF;
            farr1[i] = fval1 + late_use3 * 0.5f;
            farr2[i] = fval2 - late_use3 * 0.3f;
            
            /* Update temp variables for next iteration */
            temp_int = (temp_int * 1103515245 + 12345) & 0x7FFFFFFF;
            temp_float = temp_int * 0.0000001f;
        }
        
        /* Volatile read to prevent optimization */
        seed = temp_int;
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink = arr1[0] + arr2[0] + (int)farr1[0] + (int)farr2[0];
    (void)sink;
}

int main(void) 
{
    int i;
    int checksum = 0;
    
    /* Call target function multiple times */
    for (i = 0; i < 100; i++) {
        stress_sched(50);
        checksum += i;
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
