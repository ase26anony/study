/* Test case to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int *result) {
    volatile int sink;
    int arr1[SIZE];
    int arr2[SIZE];
    float farr1[SIZE];
    float farr2[SIZE];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
        farr1[i] = i * 0.5f;
        farr2[i] = i * 1.5f;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < 10; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < SIZE - 1; i++) {
            /* Create register pressure with many live values */
            int a = arr1[i-1];
            int b = arr1[i];
            int c = arr1[i+1];
            int d = arr2[i-1];
            int e = arr2[i];
            int f = arr2[i+1];
            
            float fa = farr1[i-1];
            float fb = farr1[i];
            float fc = farr1[i+1];
            float fd = farr2[i-1];
            float fe = farr2[i];
            float ff = farr2[i+1];
            
            /* Chain of dependent integer operations */
            int t1 = a * b + c;
            int t2 = d ^ e | f;
            int t3 = t1 * t2 - a;
            int t4 = t3 >> (b & 0xF);
            int t5 = t4 * 0x9E3779B9;
            
            /* Chain of dependent float operations */
            float ft1 = fa * fb + fc;
            float ft2 = fd - fe * ff;
            float ft3 = ft1 / (ft2 + 1.0f);
            float ft4 = ft3 * 3.14159f;
            float ft5 = ft4 * ft4 - ft3;
            
            /* Volatile read to create scheduling barrier */
            sink = arr1[0];
            
            /* Conditional execution with side effects */
            if ((t5 & 0xFF) > 128) {
                /* Branch 1: different operations */
                t5 = t5 * 2 + (int)(ft5 * 100.0f);
                ft5 = ft5 * 2.0f - (float)t5 * 0.01f;
                arr1[i] = t5 ^ arr2[i];
                farr1[i] = ft5 + farr2[i];
            } else {
                /* Branch 2: distinct operations */
                t5 = t5 / 2 - (int)(ft5 * 50.0f);
                ft5 = ft5 / 2.0f + (float)t5 * 0.02f;
                arr1[i] = t5 | arr2[i];
                farr1[i] = ft5 - farr2[i];
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extend live ranges) */
            int final_int = t5 + a + b + c + d + e + f;
            float final_float = ft5 + fa + fb + fc + fd + fe + ff;
            
            /* More operations extending dependency chains */
            sum += final_int * (i & 0x3);
            fsum += final_float * (i % 5);
            
            /* Write to array with complex index */
            int idx = (final_int ^ i) & (SIZE - 1);
            arr2[idx] = final_int;
            farr2[idx] = final_float;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = sum & 0xFF;
        farr1[0] = fsum * 0.001f;
    }
    
    *result = sum + (int)fsum;
}

int main() {
    int total_result = 0;
    
    /* Call target function multiple times */
    for (int i = 0; i < ITERS; i++) {
        int result;
        stress_sched(&result);
        total_result ^= result;
        
        /* Prevent loop optimization */
        if (i % 1000 == 0) {
            printf("Progress: %d/%d\n", i, ITERS);
        }
    }
    
    printf("Final checksum: %d\n", total_result);
    return total_result != 0;
}
