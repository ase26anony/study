/* test_sel_sched_dump.c
 * Designed to trigger selective scheduling RTL dumps in GCC's sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARRAY_SIZE 64
#define OUTER_ITER 10
#define INNER_ITER 1000

/* Force function to not be inlined to maintain complexity */
static void __attribute__((noinline)) 
stress_sched(int *result) 
{
    volatile int trigger_read; /* Volatile to create scheduling barriers */
    int arr1[ARRAY_SIZE];
    int arr2[ARRAY_SIZE];
    float farr1[ARRAY_SIZE];
    float farr2[ARRAY_SIZE];
    
    int i, j, k;
    
    /* Initialize arrays with pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 5 - 2;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    /* Outer loop - provides multiple scheduling regions */
    for (k = 0; k < OUTER_ITER; k++) {
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < INNER_ITER; j++) {
            int idx1 = (j * 7) % ARRAY_SIZE;
            int idx2 = (j * 11) % ARRAY_SIZE;
            int idx3 = (j * 13) % ARRAY_SIZE;
            int idx4 = (j * 17) % ARRAY_SIZE;
            
            /* Chain of dependent integer operations */
            int temp1 = arr1[idx1] * 3 + arr2[idx2];
            int temp2 = arr1[idx2] / 2 - arr2[idx1];
            int temp3 = temp1 ^ temp2;
            int temp4 = (temp3 << 3) | (temp1 >> 2);
            
            /* Mixed floating-point operations */
            float ftemp1 = farr1[idx3] * 2.7f + farr2[idx4];
            float ftemp2 = farr1[idx4] / 1.3f - farr2[idx3];
            float ftemp3 = ftemp1 * ftemp2 - ftemp1 / ftemp2;
            
            /* Volatile read creates scheduling barrier */
            trigger_read = arr1[(j * 19) % ARRAY_SIZE];
            
            /* Complex conditional with side effects in both branches */
            if ((temp4 + (int)ftemp3) > (trigger_read * 2)) {
                /* Branch 1: Different arithmetic pattern */
                arr1[idx1] = temp4 + (int)(ftemp3 * 1.5f);
                arr2[idx2] = temp2 - (int)(ftemp1 / 2.0f);
                farr1[idx3] = ftemp3 * 0.75f + ftemp2;
                
                /* Additional computation extending live ranges */
                int late_use1 = arr1[idx1] * 2 + arr2[idx2];
                float late_use2 = farr1[idx3] * 3.14f;
                
                /* Inline assembly as scheduling boundary */
                asm volatile("" : : : "memory");
                
                /* Use values computed much earlier */
                arr1[idx4] = late_use1 + temp3;
                farr2[idx4] = late_use2 + ftemp1;
            } else {
                /* Branch 2: Alternative computation pattern */
                arr1[idx2] = temp1 - (int)(ftemp2 * 2.5f);
                arr2[idx1] = temp4 / 2 + (int)(ftemp3 * 0.8f);
                farr2[idx3] = ftemp1 * 1.25f - ftemp3;
                
                /* Different late-use pattern */
                int late_use1 = arr1[idx2] | arr2[idx1];
                float late_use2 = farr2[idx3] / 2.71f;
                
                /* Another inline assembly barrier */
                asm volatile("" : : : "memory");
                
                /* Cross-branch value usage */
                arr2[idx4] = late_use1 ^ temp2;
                farr1[idx4] = late_use2 * ftemp2;
            }
            
            /* Post-conditional computation using values from both branches */
            int final_idx = (idx1 + idx2 + idx3) % ARRAY_SIZE;
            arr1[final_idx] = arr1[idx1] + arr2[idx2] - arr1[idx2] + arr2[idx1];
            farr1[final_idx] = farr1[idx3] * farr2[idx4] - farr1[idx4] * farr2[idx3];
            
            /* Another volatile write barrier */
            volatile int trigger_write = arr1[final_idx];
            (void)trigger_write; /* Suppress unused warning */
        }
        
        /* Inter-loop computation to create outer-loop dependencies */
        for (i = 0; i < ARRAY_SIZE / 2; i++) {
            arr1[i] = arr1[i] + arr2[ARRAY_SIZE - i - 1];
            arr2[i] = arr2[i] - arr1[ARRAY_SIZE - i - 1];
            farr1[i] = farr1[i] + farr2[ARRAY_SIZE - i - 1];
            farr2[i] = farr2[i] - farr1[ARRAY_SIZE - i - 1];
        }
    }
    
    /* Compute final result checksum */
    int sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    *result = sum;
}

/* Secondary complex function to increase scheduling opportunities */
static void __attribute__((noinline))
secondary_sched(int *arr, int size, int iter)
{
    int i, j;
    volatile int barrier;
    
    for (j = 0; j < iter; j++) {
        /* Data-dependent addressing */
        int base = (j * 53) % size;
        
        for (i = 0; i < size; i++) {
            int idx = (base + i * 31) % size;
            
            /* Complex dependency chain */
            int a = arr[idx];
            int b = arr[(idx + 1) % size];
            int c = arr[(idx + 2) % size];
            
            /* Multiple dependent operations */
            int t1 = (a * b) + (c << 2);
            int t2 = (a ^ b) | (c & 0xFF);
            int t3 = t1 - t2;
            int t4 = (t3 * 3) / (b + 1);
            
            /* Scheduling barrier */
            barrier = t4;
            
            /* Conditional with arithmetic in both paths */
            if ((t4 + j) & 1) {
                arr[idx] = t4 + barrier + i;
                arr[(idx + 3) % size] = t1 - t2 + j;
            } else {
                arr[idx] = t4 - barrier + i;
                arr[(idx + 3) % size] = t1 + t2 - j;
            }
            
            /* Late use of early computed values */
            arr[(idx + 4) % size] = t3 + arr[idx];
        }
    }
}

int main(void)
{
    int result1, result2;
    int arr[ARRAY_SIZE];
    int i;
    
    /* Initialize with non-uniform pattern */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr[i] = i * i - i + 1;
    }
    
    /* Seed for variability */
    srand(time(NULL));
    
    /* Call primary scheduling-stress function multiple times */
    for (i = 0; i < 3; i++) {
        stress_sched(&result1);
        printf("Iteration %d, result1 = %d\n", i, result1);
        
        /* Modify array for secondary function */
        arr[i % ARRAY_SIZE] = result1 % 1000;
        
        secondary_sched(arr, ARRAY_SIZE, 50 + (rand() % 100));
    }
    
    /* Final checksum */
    result2 = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        result2 += arr[i];
    }
    
    printf("Final result2 = %d\n", result2);
    printf("Total = %d\n", result1 + result2);
    
    return 0;
}
