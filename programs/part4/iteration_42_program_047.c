/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
__attribute__((noinline,noipa))
static void stress_sched(int iterations, int *result) {
    /* Local arrays to create register pressure */
    volatile int arr1[32];
    int arr2[32];
    float farr1[16];
    double darr1[16];
    
    /* Initialize arrays to prevent constant propagation */
    for (int i = 0; i < 32; i++) {
        arr1[i] = (i * 3) % 7;
        arr2[i] = (i * 5) % 11;
    }
    for (int i = 0; i < 16; i++) {
        farr1[i] = (i * 1.5f) / 7.0f;
        darr1[i] = (i * 2.3) / 5.0;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Chain of dependent integer operations */
            int a = arr1[i-1] + arr2[i];
            int b = a * arr1[i] - arr2[i-1];
            int c = b ^ (arr1[i+1] << 2);
            int d = c % (arr2[i] + 1);
            
            /* Mixed floating-point operations */
            float f1 = farr1[i % 16] * 1.1f;
            float f2 = f1 + (float)d * 0.5f;
            
            double d1 = darr1[i % 16] * 1.7;
            double d2 = d1 - (double)b * 0.3;
            
            /* Conditional with side effects in both branches */
            if ((d * i + outer) % 7 < 3) {
                /* Branch 1: Different arithmetic pattern */
                arr2[i] = (a * b) / (d + 1);
                farr1[i % 16] = f2 * 2.0f - f1;
                sum += arr2[i] * 2;
                fsum += farr1[i % 16];
            } else {
                /* Branch 2: Alternative computation */
                arr2[i] = (b ^ c) | (d << 1);
                farr1[i % 16] = f1 / (f2 + 0.1f);
                sum -= arr2[i];
                fsum -= farr1[i % 16];
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* More computations using values from earlier in the loop */
            int e = arr2[i-1] + d;
            int f = e * arr1[i] >> 2;
            
            double d3 = d2 * 0.9 + (double)f;
            darr1[i % 16] = d3 - d1 * 0.5;
            
            /* Use values computed much earlier */
            arr1[i] = (arr1[i] + f) % 256;
            dsum += darr1[i % 16];
            
            /* Additional volatile access for scheduling complexity */
            volatile int barrier = arr1[i];
            (void)barrier;
        }
        
        /* Cross-iteration dependencies */
        arr1[0] = (arr1[31] + outer) % 127;
        arr2[0] = (arr2[31] ^ outer) & 0xFF;
    }
    
    /* Final computation to use all accumulated values */
    *result = sum + (int)fsum + (int)dsum;
}

/* Helper to prevent dead code elimination */
volatile int global_counter = 0;

int main() {
    int result1 = 0, result2 = 0;
    
    /* Call multiple times to ensure execution */
    stress_sched(100, &result1);
    global_counter++;
    
    stress_sched(50, &result2);
    global_counter++;
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d\n", result1, result2);
    printf("Checksum: %d\n", result1 + result2 + global_counter);
    
    return 0;
}
