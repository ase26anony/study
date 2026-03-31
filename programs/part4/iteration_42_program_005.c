/* test_sel_sched_dump.c
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops 
 *               -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic test_sel_sched_dump.c
 * Or for more aggressive scheduling: gcc -O3 -fsel-sched-pipelining -fsched-verbose=5 -mtune=core2 test_sel_sched_dump.c 2>&1
 */

#include <stdio.h>
#include <stdlib.h>

/* Force function not to be inlined to maintain complexity */
static void __attribute__((noinline)) 
stress_sched(int iterations, int *result) {
    volatile int barrier;  /* Creates scheduling barriers */
    int arr1[32], arr2[32];
    float farr1[16], farr2[16];
    int i, j, k;
    
    /* Initialize arrays with pattern to prevent dead code elimination */
    for (i = 0; i < 32; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 7;
    }
    for (i = 0; i < 16; i++) {
        farr1[i] = i * 1.5f;
        farr2[i] = i * 2.5f;
    }
    
    int sum = 0;
    barrier = 0;  /* Scheduling barrier */
    
    /* Outer loop - provides enough iterations for scheduling analysis */
    for (i = 0; i < iterations; i++) {
        int temp1 = arr1[i % 32];
        int temp2 = arr2[i % 32];
        float ftemp1 = farr1[i % 16];
        float ftemp2 = farr2[i % 16];
        
        /* Complex inner loop with high ILP potential */
        for (j = 0; j < 128; j++) {
            /* Chain of dependent integer operations */
            int a = temp1 * j + 12345;
            int b = temp2 * j - 54321;
            int c = a ^ b;
            int d = (c << 3) | (c >> 29);
            
            /* Chain of dependent floating-point operations */
            float fa = ftemp1 * j * 0.5f;
            float fb = ftemp2 * j * 1.5f;
            float fc = fa + fb;
            float fd = fc * 2.0f - 1.0f;
            
            /* Conditional execution with side effects */
            if ((d & 0xFF) > 128) {
                /* Branch 1: Integer-heavy operations */
                int e = d * 3 + a;
                int f = e / 7 + b;
                arr1[(j + i) % 32] = e ^ f;
                farr1[j % 16] = fd * 0.75f;
            } else {
                /* Branch 2: Different operations */
                int e = d / 5 - a;
                int f = e * 11 + b;
                arr2[(j + i) % 32] = e | f;
                farr2[j % 16] = fd * 1.25f;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extending live ranges) */
            int late_use1 = a + d;
            int late_use2 = b ^ c;
            float late_use3 = fa * fb;
            
            /* More operations that use extended live ranges */
            if (j % 3 == 0) {
                arr1[(late_use1 + j) % 32] = late_use2;
                farr1[j % 16] += late_use3;
            } else if (j % 3 == 1) {
                arr2[(late_use2 + j) % 32] = late_use1;
                farr2[j % 16] -= late_use3;
            } else {
                /* Mix integer and float operations */
                int mixed = late_use1 * (int)late_use3;
                float fmixed = late_use2 * 0.01f;
                arr1[j % 32] = mixed;
                farr2[j % 16] = fmixed;
            }
            
            /* Another scheduling barrier */
            barrier = j;
        }
        
        /* Accumulate results to prevent optimization */
        sum += arr1[i % 32] + arr2[i % 32];
        sum += (int)farr1[i % 16] + (int)farr2[i % 16];
        
        /* Volatile read creates another scheduling point */
        temp1 = barrier;
    }
    
    *result = sum;
}

/* Secondary complex function to increase scheduling complexity */
static void __attribute__((noinline))
helper_func(int *data, int size) {
    int i;
    volatile int v = 0;
    
    for (i = 0; i < size; i++) {
        /* Complex addressing patterns */
        int idx1 = (i * 37) % size;
        int idx2 = (i * 13) % size;
        int idx3 = (i * 29) % size;
        
        /* Multiple dependent operations */
        int a = data[idx1] * 3;
        int b = data[idx2] + 7;
        int c = data[idx3] - 11;
        
        /* Conditional with arithmetic in both branches */
        if ((a ^ b) > c) {
            data[idx1] = a * b - c;
            asm volatile("" ::: "memory");
        } else {
            data[idx2] = b / 2 + a;
            data[idx3] = c * 3 - b;
        }
        
        v = i;  /* Scheduling barrier */
    }
}

int main() {
    int result1, result2;
    int data[64];
    int i;
    
    /* Initialize data array */
    for (i = 0; i < 64; i++) {
        data[i] = i * 17 + 23;
    }
    
    /* Call both complex functions multiple times */
    for (i = 0; i < 3; i++) {
        stress_sched(100, &result1);
        helper_func(data, 64);
    }
    
    /* Compute final checksum */
    int checksum = result1;
    for (i = 0; i < 64; i++) {
        checksum += data[i];
    }
    
    printf("Result: %d\n", checksum);
    return 0;
}
