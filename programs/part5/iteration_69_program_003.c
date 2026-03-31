/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERATIONS 100000

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with many live variables */
__attribute__((noinline))
static void high_pressure_loop(int *arr1, int *arr2, float *farr1, float *farr2) {
    int i;
    /* Many independent integer operations to create scheduling candidates */
    int r0 = arr1[0], r1 = arr1[1], r2 = arr1[2], r3 = arr1[3];
    int r4 = arr1[4], r5 = arr1[5], r6 = arr1[6], r7 = arr1[7];
    int r8, r9, r10, r11, r12, r13, r14, r15;
    
    float f0 = farr1[0], f1 = farr1[1], f2 = farr1[2], f3 = farr1[3];
    float f4 = farr1[4], f5 = farr1[1], f6 = farr1[2], f7 = farr1[3];
    float f8, f9, f10, f11, f12, f13, f14, f15;
    
    /* Create independent instruction groups - scheduler will have multiple candidates */
    r8 = r0 + r1;      /* Group 1 - independent adds */
    r9 = r2 + r3;
    r10 = r4 + r5;
    r11 = r6 + r7;
    
    f8 = f0 * f1;      /* Group 2 - independent multiplies */
    f9 = f2 * f3;
    f10 = f4 * f5;
    f11 = f6 * f7;
    
    /* Mix operations to create different priorities */
    r12 = r8 * vol_a;  /* Volatile creates memory dependency */
    r13 = r9 / (vol_b + 1);  /* Division has higher latency */
    r14 = r10 - r11;
    r15 = r12 ^ r13;   /* XOR operation */
    
    /* Floating point divisions - long latency operations */
    f12 = f8 / vol_f1;
    f13 = f9 / vol_f2;
    f14 = f10 * f11;
    f15 = f12 + f13;
    
    /* Store results creating register pressure release */
    arr2[0] = r12 + r14;
    arr2[1] = r13 + r15;
    arr2[2] = r8 ^ r10;
    arr2[3] = r9 | r11;
    
    farr2[0] = f12 + f14;
    farr2[1] = f13 + f15;
    farr2[2] = f8 * f10;
    farr2[3] = f9 / f11;
    
    /* Inline assembly to clobber registers and force spills */
    asm volatile (
        "movl $0, %%eax\n"
        "movl $0, %%ebx\n"
        "movl $0, %%ecx\n"
        "movl $0, %%edx\n"
        "movl $0, %%esi\n"
        "movl $0, %%edi\n"
        : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
}

/* Function with artificial dependencies and delays */
__attribute__((noinline))
static void mixed_dependency(int *arr, float *farr) {
    int i;
    int t0 = arr[0], t1 = arr[1], t2 = arr[2], t3 = arr[3];
    float ft0 = farr[0], ft1 = farr[1], ft2 = farr[2], ft3 = farr[3];
    
    /* Chain of dependencies causing delays */
    for (i = 0; i < 8; i++) {
        t0 = t0 * vol_c + i;      /* Volatile read creates delay */
        t1 = t1 / (vol_d + 1);    /* Division + volatile = longer delay */
        t2 = t2 ^ t0;
        t3 = t3 | t1;
        
        /* Floating point with volatile - mixed resource competition */
        ft0 = ft0 / vol_f3;       /* FP division - high latency */
        ft1 = ft1 * ft0;
        ft2 = ft2 + vol_f1;       /* Volatile FP read */
        ft3 = ft3 - ft2;
        
        /* Memory barrier to force ordering */
        asm volatile ("" : : : "memory");
    }
    
    /* Store results */
    arr[0] = t0 + t2;
    arr[1] = t1 * t3;
    arr[2] = t0 ^ t1;
    arr[3] = t2 | t3;
    
    farr[0] = ft0 + ft2;
    farr[1] = ft1 * ft3;
    farr[2] = ft0 / ft1;
    farr[3] = ft2 - ft3;
}

/* Function with completely independent instructions for candidate selection */
__attribute__((noinline))
static void independent_instructions(int *results) {
    /* 16 independent calculations - scheduler has many candidates */
    int a1 = vol_a * 2;
    int a2 = vol_b + 3;
    int a3 = vol_c / 2;
    int a4 = vol_d - 1;
    int a5 = a1 ^ a2;
    int a6 = a3 & a4;
    int a7 = a1 | a3;
    int a8 = a2 ^ a4;
    
    float f1 = vol_f1 * 2.0f;
    float f2 = vol_f2 + 3.14f;
    float f3 = vol_f3 / 2.0f;
    float f4 = f1 * f2;
    float f5 = f2 / f3;
    float f6 = f1 + f3;
    float f7 = f4 - f5;
    float f8 = f6 * f7;
    
    /* Use all results to prevent elimination */
    results[0] = a5 + a6 + (int)f1;
    results[1] = a7 ^ a8 + (int)f2;
    results[2] = (a1 * a4) | (int)f3;
    results[3] = (a2 + a3) & (int)f4;
    results[4] = (int)(f5 * 100);
    results[5] = (int)(f6 + f7);
    results[6] = (int)f8;
    results[7] = a5 ^ (int)f8;
}

/* Main computational kernel */
__attribute__((noinline))
static int compute_kernel(int seed) {
    int i;
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    int results[8];
    int total = seed;
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (i * 3 + seed) % 100;
        farr1[i] = (float)((i * 5 + seed) % 100) / 10.0f;
    }
    
    /* Perform multiple scheduling-intensive operations */
    for (i = 0; i < ITERATIONS / 100; i++) {
        high_pressure_loop(arr1 + (i % (SIZE - 8)), 
                          arr2 + (i % (SIZE - 8)),
                          farr1 + (i % (SIZE - 8)),
                          farr2 + (i % (SIZE - 8)));
        
        mixed_dependency(arr2 + (i % (SIZE - 8)),
                        farr2 + (i % (SIZE - 8)));
        
        independent_instructions(results);
        
        /* Accumulate results to prevent dead code elimination */
        total += arr2[i % (SIZE - 8)] + (int)farr2[i % (SIZE - 8)];
        total += results[i % 8];
    }
    
    return total;
}

int main() {
    int i;
    int final_result = 0;
    clock_t start, end;
    
    srand(time(NULL));
    
    printf("Starting scheduling test...\n");
    start = clock();
    
    /* Run kernel multiple times to ensure hot code scheduling */
    for (i = 0; i < 10; i++) {
        final_result ^= compute_kernel(i + final_result);
    }
    
    end = clock();
    
    printf("Final result: %d\n", final_result);
    printf("Time taken: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Use result to prevent optimization */
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
