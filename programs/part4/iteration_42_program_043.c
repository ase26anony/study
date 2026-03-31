/* Test program to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure function complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) {
    volatile int barrier = 0;
    int arr1[32], arr2[32];
    float farr1[32], farr2[32];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 5;
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    float fsum = 0.0f;
    
    /* Outer loop to provide sufficient iterations */
    for (int outer = 0; outer < iterations; outer++) {
        /* Complex inner loop with high ILP potential */
        for (int i = 1; i < 31; i++) {
            /* Create register pressure with many live variables */
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
            
            /* Chain of dependent arithmetic operations */
            int t1 = a * b + c;
            int t2 = d - e * f;
            int t3 = t1 ^ t2;
            int t4 = t3 << (i & 3);
            
            float ft1 = fa * fb + fc;
            float ft2 = fd - fe * ff;
            float ft3 = ft1 * ft2;
            float ft4 = ft3 / (1.0f + (i & 7));
            
            /* Volatile read/write to create scheduling barriers */
            barrier = t4;
            volatile int dummy = barrier;
            
            /* Conditional execution with side effects */
            if ((t4 & 0xF) > (i & 0xF)) {
                /* Branch 1: different arithmetic pattern */
                arr1[i] = t4 * 7 - t3;
                arr2[i] = (t2 << 2) | (t1 & 0xFF);
                farr1[i] = ft4 * 3.14159f;
                farr2[i] = ft3 / 2.71828f;
                
                /* More computations in this branch */
                int extra = (t4 * i) / (1 + (i & 3));
                arr1[i] += extra;
                farr1[i] += extra * 0.01f;
            } else {
                /* Branch 2: alternative computation pattern */
                arr1[i] = (t3 * 11) >> (i & 1);
                arr2[i] = t4 ^ (t2 * 13);
                farr1[i] = ft3 * 1.41421f;
                farr2[i] = ft4 / 1.73205f;
                
                /* Different extra computations */
                float fextra = ft4 * i * 0.12345f;
                farr2[i] += fextra;
                arr2[i] += (int)fextra;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extended live ranges) */
            int late_use1 = t1 + t2 + t3 + t4;
            float late_use2 = ft1 + ft2 + ft3 + ft4;
            
            /* More computations using extended live values */
            arr1[(i + 3) % 32] += late_use1 & 0xFF;
            arr2[(i + 5) % 32] ^= late_use1 >> 8;
            farr1[(i + 2) % 32] += late_use2 * 0.1f;
            farr2[(i + 4) % 32] -= late_use2 * 0.01f;
            
            /* Accumulate results to prevent dead code elimination */
            sum += arr1[i] + arr2[i];
            fsum += farr1[i] + farr2[i];
        }
        
        /* Mix array elements between outer iterations */
        for (int i = 0; i < 16; i++) {
            int tmp = arr1[i];
            arr1[i] = arr1[31 - i];
            arr1[31 - i] = tmp;
            
            float ftmp = farr1[i];
            farr1[i] = farr1[31 - i];
            farr1[31 - i] = ftmp;
        }
    }
    
    /* Final computation mixing integer and float results */
    *result = sum + (int)fsum;
}

int main() {
    int result = 0;
    
    /* Call multiple times with different iteration counts */
    stress_sched(100, &result);
    stress_sched(50, &result);
    stress_sched(75, &result);
    
    printf("Result: %d\n", result);
    
    /* Additional test with different patterns */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    /* Another complex loop pattern */
    for (int i = 0; i < 99; i++) {
        for (int j = 0; j < 10; j++) {
            arr[i] = arr[i] * 3 + arr[i+1];
            arr[i+1] = arr[i] ^ (arr[i+1] << 1);
            
            /* More scheduling complexity */
            if ((arr[i] & 0x7) == 0) {
                asm volatile("" ::: "memory");
                arr[i] = ~arr[i];
            } else {
                arr[i] = arr[i] * 0x5A5A5A5A;
            }
        }
    }
    
    int checksum = 0;
    for (int i = 0; i < 100; i++) {
        checksum ^= arr[i];
    }
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
