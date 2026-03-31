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

/* Function to create high register pressure with many live variables */
__attribute__((noinline))
void high_pressure_loop(int *arr1, int *arr2, float *farr1, float *farr2) {
    int i;
    /* Many independent integer variables to create register pressure */
    int r0, r1, r2, r3, r4, r5, r6, r7, r8, r9;
    int r10, r11, r12, r13, r14, r15, r16, r17, r18, r19;
    float f0, f1, f2, f3, f4, f5, f6, f7, f8, f9;
    
    /* Initialize with volatile reads to create dependencies */
    r0 = vol_a;
    r1 = vol_b;
    f0 = vol_f1;
    f1 = vol_f2;
    
    /* Unrolled loop with many independent operations */
    for (i = 0; i < ARRAY_SIZE - 8; i += 8) {
        /* Group 1: Independent integer operations */
        r2 = arr1[i] + r0;
        r3 = arr1[i+1] * r1;
        r4 = arr1[i+2] - r0;
        r5 = arr1[i+3] / (r1 + 1);
        
        /* Group 2: More independent operations */
        r6 = arr1[i+4] & r2;
        r7 = arr1[i+5] | r3;
        r8 = arr1[i+6] ^ r4;
        r9 = arr1[i+7] << 2;
        
        /* Group 3: Floating point operations (different functional units) */
        f2 = farr1[i] * f0;
        f3 = farr1[i+1] / f1;  /* Division has higher latency */
        f4 = farr1[i+2] + f2;
        f5 = farr1[i+3] - f3;
        
        /* Group 4: Mixed operations with dependencies */
        r10 = (int)(f2 * 100.0f) + r2;
        r11 = (int)(f3 * 100.0f) + r3;
        f6 = (float)r4 * 0.01f;
        f7 = (float)r5 * 0.01f;
        
        /* Store results with volatile write to prevent reordering */
        arr2[i] = r2 + vol_c;
        arr2[i+1] = r3 + vol_d;
        farr2[i] = f2 + vol_f3;
        farr2[i+1] = f3 * vol_f1;
        
        /* More stores to create memory pressure */
        arr2[i+2] = r4;
        arr2[i+3] = r5;
        arr2[i+4] = r6;
        arr2[i+5] = r7;
        farr2[i+2] = f4;
        farr2[i+3] = f5;
        
        /* Cross-iteration dependencies to force ordering */
        r0 = r10 >> 1;
        r1 = r11 << 1;
        f0 = f6 * 0.5f;
        f1 = f7 * 2.0f;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n"
            "movl $0, %%ebx\n"
            "movl $0, %%ecx\n"
            "movl $0, %%edx\n"
            "movl $0, %%esi\n"
            "movl $0, %%edi\n"
            :
            :
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with artificial resource conflicts and delays */
__attribute__((noinline))
void mixed_dependency(int *arr, float *farr) {
    int i;
    /* Variables with different lifetimes to create priority differences */
    int early1, early2, late1, late2;
    float flow1, flow2, fhigh1, fhigh2;
    
    /* Long latency chain */
    early1 = vol_a * 2;
    early2 = vol_b * 3;
    
    /* Independent short operations that could be scheduled earlier */
    for (i = 0; i < 16; i++) {
        int temp1 = arr[i] + i;
        int temp2 = arr[i+16] * i;
        float ftemp1 = farr[i] * (float)i;
        float ftemp2 = farr[i+16] / (float)(i + 1);  /* Division has latency */
        
        /* Mix operations to compete for functional units */
        arr[i] = temp1 + early1;
        arr[i+16] = temp2 - early2;
        farr[i] = ftemp1 + 1.0f;
        farr[i+16] = ftemp2 - 1.0f;
        
        /* Volatile access creates a hard barrier */
        if (i % 4 == 0) {
            early1 += vol_c;
            early2 += vol_d;
        }
    }
    
    /* Create delay with serial dependencies */
    late1 = early1;
    for (i = 0; i < 8; i++) {
        late1 = late1 * 2 + arr[i];
    }
    
    late2 = early2;
    for (i = 0; i < 8; i++) {
        late2 = late2 / 2 + arr[i+8];  /* Division in loop creates delays */
    }
    
    /* Floating point with different latencies */
    flow1 = vol_f1;
    for (i = 0; i < 4; i++) {
        flow1 = flow1 * 1.5f + farr[i];  /* Multiplication */
    }
    
    fhigh1 = vol_f2;
    for (i = 0; i < 4; i++) {
        fhigh1 = fhigh1 / 1.3f + farr[i+4];  /* Division - higher latency */
    }
    
    /* Final results */
    arr[0] = late1 + late2;
    farr[0] = flow1 + fhigh1;
}

/* Function with many independent instructions for candidate selection */
__attribute__((noinline))
void independent_instructions(int *arr1, int *arr2, int *arr3) {
    /* Many independent operations that can be reordered */
    int t1 = arr1[0] + arr1[1];
    int t2 = arr1[2] * arr1[3];
    int t3 = arr1[4] - arr1[5];
    int t4 = arr1[6] & arr1[7];
    int t5 = arr1[8] | arr1[9];
    int t6 = arr1[10] ^ arr1[11];
    int t7 = arr1[12] << 2;
    int t8 = arr1[13] >> 1;
    int t9 = arr1[14] + 1;
    int t10 = arr1[15] - 1;
    
    /* Another independent group */
    int u1 = arr2[0] * 2;
    int u2 = arr2[1] / 2;
    int u3 = arr2[2] + 100;
    int u4 = arr2[3] - 100;
    int u5 = arr2[4] & 0xFF;
    int u6 = arr2[5] | 0xFF00;
    int u7 = arr2[6] ^ 0xFFFF;
    int u8 = arr2[7] << 3;
    int u9 = arr2[8] >> 3;
    int u10 = arr2[9] * 3;
    
    /* Cross-group operations with some dependencies */
    arr3[0] = t1 + u1;
    arr3[1] = t2 - u2;
    arr3[2] = t3 * u3;
    arr3[3] = t4 & u4;
    arr3[4] = t5 | u5;
    arr3[5] = t6 ^ u6;
    arr3[6] = t7 + u7;
    arr3[7] = t8 - u8;
    arr3[8] = t9 * u9;
    arr3[9] = t10 / (u10 + 1);
    
    /* Volatile write to prevent dead code elimination */
    vol_a = arr3[0];
}

int main() {
    int *arr1, *arr2, *arr3;
    float *farr1, *farr2;
    int i;
    long long sum = 0;
    
    /* Allocate and initialize arrays */
    arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = 0;
        farr1[i] = (float)(rand() % 1000) / 10.0f;
        farr2[i] = 0.0f;
    }
    
    /* Perform many iterations to make scheduling important */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(arr1, arr2, farr1, farr2);
        mixed_dependency(arr1, farr1);
        independent_instructions(arr1, arr2, arr3);
        
        /* Update volatile variables to change dependencies */
        if (i % 1000 == 0) {
            vol_a = (vol_a * 3) % 100;
            vol_b = (vol_b * 5) % 100;
            vol_c = (vol_c + 1) % 100;
            vol_d = (vol_d - 1) % 100;
            vol_f1 = vol_f1 * 1.1f;
            vol_f2 = vol_f2 / 1.1f;
            vol_f3 = vol_f3 + 0.1f;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < ARRAY_SIZE; i++) {
        sum += arr1[i] + arr2[i] + arr3[i] + (int)farr1[i] + (int)farr2[i];
    }
    
    printf("Checksum: %lld\n", sum);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    
    return 0;
}
