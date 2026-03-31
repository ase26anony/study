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
void high_pressure_loop(int *arr1, int *arr2, int *arr3, float *farr1, float *farr2) {
    int i;
    /* Many independent integer operations to create scheduling candidates */
    int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    int t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    float ft1, ft2, ft3, ft4, ft5, ft6, ft7, ft8, ft9, ft10;
    
    /* Create artificial dependencies with volatile */
    t1 = vol_a + 1;
    t2 = vol_b * 2;
    t3 = vol_c - 1;
    
    /* Group 1: Independent integer operations (candidates for scheduler) */
    t4 = arr1[0] + arr1[1];
    t5 = arr1[2] * arr1[3];
    t6 = arr1[4] - arr1[5];
    t7 = arr1[6] & arr1[7];
    t8 = arr1[8] | arr1[9];
    t9 = arr1[10] ^ arr1[11];
    t10 = arr1[12] << 2;
    
    /* Group 2: More independent operations */
    t11 = arr2[0] + t1;
    t12 = arr2[1] * t2;
    t13 = arr2[2] - t3;
    t14 = arr2[3] & t4;
    t15 = arr2[4] | t5;
    t16 = arr2[5] ^ t6;
    t17 = arr2[6] << 1;
    t18 = arr2[7] >> 2;
    
    /* Floating point operations to create different functional unit pressure */
    ft1 = farr1[0] * farr2[0];
    ft2 = farr1[1] / farr2[1];  /* Long latency divide */
    ft3 = farr1[2] + farr2[2];
    ft4 = farr1[3] - farr2[3];
    
    /* More FP ops */
    ft5 = ft1 * vol_f1;
    ft6 = ft2 / vol_f2;  /* Another divide for latency */
    ft7 = ft3 + ft4;
    ft8 = ft5 - ft6;
    
    /* Mix integer and float with dependencies */
    t19 = (int)ft7 + t7;
    t20 = (int)ft8 * t8;
    
    /* Use results to prevent elimination */
    arr3[0] = t9 + t10 + t11 + t12;
    arr3[1] = t13 + t14 + t15 + t16;
    arr3[2] = t17 + t18 + t19 + t20;
    farr1[10] = ft7 + ft8;
    
    /* Inline assembly to clobber registers and increase pressure */
    asm volatile (
        "mov $0, %%eax\n"
        "mov $0, %%ebx\n"
        "mov $0, %%ecx\n"
        "mov $0, %%edx\n"
        "mov $0, %%esi\n"
        "mov $0, %%edi\n"
        : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
void mixed_dependency(int *arr, int n) {
    int i, j;
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    int prod1 = 1, prod2 = 1;
    
    /* Loop with breaks creates different instruction priorities */
    for (i = 0; i < n; i++) {
        /* Independent chains within loop */
        int val = arr[i];
        
        /* Chain 1: Simple arithmetic */
        int t1 = val + vol_a;
        int t2 = t1 * 2;
        int t3 = t2 - vol_b;
        
        /* Chain 2: Different operations */
        int t4 = val & 0xFF;
        int t5 = t4 | 0x80;
        int t6 = t5 ^ 0x55;
        
        /* Chain 3: Memory access pattern */
        int t7 = arr[(i + 1) % n];
        int t8 = t7 + val;
        int t9 = t8 * 3;
        
        /* Conditional creates priority differences */
        if (val % 3 == 0) {
            sum1 += t3;
            prod1 *= t6;
            /* Volatile access creates delay */
            asm volatile("" : : "r"(vol_c) : "memory");
        } else if (val % 3 == 1) {
            sum2 += t6;
            prod2 *= t9;
            /* Another volatile for delay */
            asm volatile("" : : "r"(vol_d) : "memory");
        } else {
            sum3 += t9;
            sum4 += t3 + t6;
        }
        
        /* Long latency operation every 8 iterations */
        if (i % 8 == 0) {
            float fval = (float)val;
            float fdiv = vol_f1 / (fval + 1.0f);  /* Floating divide */
            sum1 += (int)fdiv;
        }
    }
    
    /* Use results */
    arr[0] = sum1 + sum2 + sum3 + sum4;
    arr[1] = prod1 + prod2;
}

/* Function with many independent statements for candidate array */
__attribute__((noinline))
void independent_instructions(int *out) {
    /* Many independent operations - scheduler will have many candidates */
    int a1 = vol_a * 2;
    int a2 = vol_b + 3;
    int a3 = vol_c - 1;
    int a4 = vol_d / 2;
    
    int b1 = a1 * a2;
    int b2 = a3 + a4;
    int b3 = a1 & a2;
    int b4 = a3 | a4;
    
    int c1 = b1 << 2;
    int c2 = b2 >> 1;
    int c3 = b3 ^ b4;
    int c4 = b1 % (b2 + 1);
    
    int d1 = c1 + c2;
    int d2 = c3 * c4;
    int d3 = c1 & c2;
    int d4 = c3 | c4;
    
    int e1 = d1 - d2;
    int e2 = d3 + d4;
    int e3 = d1 ^ d2;
    int e4 = d3 & d4;
    
    /* Use all results to prevent elimination */
    out[0] = a1 + a2 + a3 + a4;
    out[1] = b1 + b2 + b3 + b4;
    out[2] = c1 + c2 + c3 + c4;
    out[3] = d1 + d2 + d3 + d4;
    out[4] = e1 + e2 + e3 + e4;
}

int main() {
    int i;
    int *arr1, *arr2, *arr3, *tmp;
    float *farr1, *farr2;
    int result[5] = {0};
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Allocate and initialize arrays */
    arr1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    arr3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    tmp = (int*)malloc(ARRAY_SIZE * sizeof(int));
    farr1 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    farr2 = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !arr3 || !tmp || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with random data */
    for (i = 0; i < ARRAY_SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = 0;
        tmp[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 10.0f;
        farr2[i] = (float)(rand() % 1000) / 10.0f;
    }
    
    printf("Starting scheduling test...\n");
    
    /* Main computation loop - this is where scheduling happens */
    for (i = 0; i < ITERATIONS; i++) {
        /* Call functions that create scheduling scenarios */
        high_pressure_loop(arr1, arr2, arr3, farr1, farr2);
        
        if (i % 100 == 0) {
            mixed_dependency(tmp, ARRAY_SIZE / 4);
        }
        
        if (i % 50 == 0) {
            independent_instructions(result);
        }
        
        /* Update volatile to create new dependencies */
        if (i % 1000 == 0) {
            vol_a = (vol_a * 3) % 100;
            vol_b = (vol_b + 5) % 100;
            vol_f1 = vol_f1 * 1.01f;
        }
    }
    
    /* Compute checksum to use all results */
    int checksum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i] + tmp[i];
        checksum += (int)farr1[i] + (int)farr2[i];
    }
    for (i = 0; i < 5; i++) {
        checksum += result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Test completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(tmp);
    free(farr1);
    free(farr2);
    
    return 0;
}
