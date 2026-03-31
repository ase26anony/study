/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define ITERATIONS 100000
#define ARRAY_SIZE 1024

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(int *arr1, int *arr2, float *farr1, float *farr2) {
    int i;
    /* Many independent integer operations to create scheduling candidates */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    
    /* Unrolled loop creates many live temporaries */
    for (i = 0; i < ARRAY_SIZE - 10; i += 10) {
        /* Group 1: Independent integer operations */
        r0 = arr1[i] + arr2[i];
        r1 = arr1[i+1] * arr2[i+1];
        r2 = arr1[i+2] - arr2[i+2];
        r3 = arr1[i+3] & arr2[i+3];
        r4 = arr1[i+4] | arr2[i+4];
        
        /* Group 2: More independent operations */
        r5 = arr1[i+5] ^ arr2[i+5];
        r6 = arr1[i+6] << 2;
        r7 = arr1[i+7] >> 1;
        r8 = arr1[i+8] + vol_a;  /* Volatile dependency creates delay */
        r9 = arr1[i+9] * vol_b;
        
        /* Group 3: Floating point operations (different functional units) */
        f0 = farr1[i] * farr2[i];
        f1 = farr1[i+1] / farr2[i+1];  /* FP divide has high latency */
        f2 = farr1[i+2] + farr2[i+2];
        f3 = farr1[i+3] - farr2[i+3];
        f4 = sqrtf(farr1[i+4]);        /* Math function call */
        
        /* Group 4: Mixed operations with dependencies */
        r10 = r0 + r1;
        r11 = r2 * r3;
        r12 = r4 ^ r5;
        r13 = r6 + r7;
        f5 = f0 * f1;
        
        /* Create artificial resource conflicts with inline asm */
        asm volatile ("" : : "r"(r8), "r"(r9), "r"(r10), "r"(r11), 
                      "r"(r12), "r"(r13) : "memory");
        
        /* Store results to prevent elimination */
        arr1[i] = r10 + r11;
        arr2[i+1] = r12 - r13;
        farr1[i+2] = f5 * vol_f1;
        
        /* More operations to increase pressure */
        r14 = vol_c * 2;
        r15 = vol_d / 2;
        f6 = vol_f2 * 3.14f;
        f7 = vol_f3 / 2.0f;
        
        /* Long latency chain */
        f8 = f6 * f7;
        f9 = f8 / farr2[i+5];
        farr1[i+5] = f9 + 1.0f;
        
        /* Use all variables to keep them live */
        r16 = r14 + r15;
        r17 = (int)f6 + (int)f7;
        r18 = r16 * r17;
        r19 = r18 ^ 0xFF;
        
        arr2[i+8] = r19;
    }
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
int mixed_dependency(int x, int y, float z) {
    int result = 0;
    
    /* Create different priority paths */
    if (x > y) {
        /* High priority path */
        result = x * y;
        result += (int)(z * 100.0f);
        
        /* Volatile access creates scheduling barrier */
        result *= vol_a;
        
        /* Inline asm with register clobbering */
        asm volatile ("# Artificial dependency" : : : "eax", "ebx", "ecx", "edx");
    } else {
        /* Lower priority path */
        result = x + y;
        result -= (int)(z * 50.0f);
        
        /* More operations with dependencies */
        int t1 = result * 2;
        int t2 = t1 + vol_b;
        int t3 = t2 / 3;
        result = t3 ^ 0xABCD;
    }
    
    /* Common path with independent operations */
    int a = result + 1000;
    int b = result - 500;
    int c = a * b;
    int d = c / result;
    
    /* Force register pressure at end */
    asm volatile ("" : : "r"(a), "r"(b), "r"(c), "r"(d) : "memory");
    
    return d + result;
}

/* Function with memory aliasing to create delays */
__attribute__((noinline))
void memory_aliasing_ops(int *ptr1, int *ptr2, float *fptr) {
    /* Assume possible aliasing */
    *ptr1 = *ptr2 + 1;
    
    /* Memory barrier */
    asm volatile ("" : : : "memory");
    
    /* Dependent operations */
    int temp = *ptr1 * 2;
    
    /* Floating point with possible dependency */
    float ftemp = *fptr * 3.14f;
    
    /* Store with potential aliasing */
    *ptr2 = temp + (int)ftemp;
    
    /* More operations */
    for (int i = 0; i < 8; i++) {
        ptr1[i] = ptr2[i] + i;
        fptr[i] = fptr[i] * 1.1f;
    }
}

/* Main computational kernel */
__attribute__((hot))
void compute_kernel(int *data1, int *data2, float *fdata1, float *fdata2) {
    int sum = 0;
    
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call high pressure function - creates many scheduling candidates */
        high_pressure_loop(data1, data2, fdata1, fdata2);
        
        /* Call mixed dependency function - creates priority differences */
        int idx = iter % ARRAY_SIZE;
        sum += mixed_dependency(data1[idx], data2[idx], fdata1[idx]);
        
        /* Memory operations with potential delays */
        if (iter % 100 == 0) {
            memory_aliasing_ops(&data1[idx], &data2[idx], &fdata1[idx]);
        }
        
        /* Prevent loop unrolling from simplifying too much */
        asm volatile ("" : : : "memory");
    }
    
    /* Use result to prevent dead code elimination */
    vol_a = sum % 1000;
}

int main() {
    /* Allocate and initialize data */
    int *data1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *data2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    float *fdata1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *fdata2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!data1 || !data2 || !fdata1 || !fdata2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data1[i] = rand() % 1000;
        data2[i] = rand() % 1000;
        fdata1[i] = (float)(rand() % 1000) / 10.0f;
        fdata2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    printf("Starting computation...\n");
    
    /* Run the kernel where scheduling happens */
    compute_kernel(data1, data2, fdata1, fdata2);
    
    /* Compute checksum to verify correctness */
    int checksum = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += data1[i] + data2[i] + (int)fdata1[i];
    }
    
    printf("Computation complete. Checksum: %d\n", checksum);
    printf("vol_a = %d, vol_b = %d\n", vol_a, vol_b);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(fdata1);
    free(fdata2);
    
    return 0;
}
