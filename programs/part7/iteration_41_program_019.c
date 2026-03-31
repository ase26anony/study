/* sel-sched-trigger.c
 * Designed to trigger GCC's selective scheduler debug output
 * Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-verbose=5 -fdump-rtl-all -fno-schedule-insns -fno-schedule-insns2 -o trigger sel-sched-trigger.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining and create more complex CFG */
extern int external_helper(int x);

/* Volatile variables to prevent compile-time optimization */
volatile int g_volatile_seed = 42;
volatile float g_volatile_threshold = 0.5f;

/* Function with complex loop structure to engage selective scheduler */
int process_data(int* arr1, int* arr2, float* farr1, float* farr2, int n, int iter) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop - gives scheduler repeated region to analyze */
    for (int outer = 0; outer < iter; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency on acc */
            int temp = arr1[i] * arr2[i-1];
            acc += temp;
            
            /* Mixed integer/float operations */
            float f_val = farr1[i] * 2.0f + (float)acc * 0.001f;
            
            /* Conditional control flow inside loop - creates basic block boundaries */
            if (f_val > g_volatile_threshold) {
                /* Complex floating point operation */
                farr2[i] = sqrtf(fabsf(f_val)) + farr2[i-1] * 0.8f;
                
                /* Bitwise operations mixed with float conversion */
                arr1[i] = (arr1[i] & 0xFF) | ((int)(farr2[i] * 100.0f) << 8);
            } else {
                /* Alternative path with different operations */
                farr2[i] = powf(f_val, 1.5f) - farr1[i-1];
                arr1[i] = (arr1[i] ^ 0xAA) + (int)(farr2[i] * 50.0f);
            }
            
            /* More mixed operations */
            f_acc += farr2[i] * 0.1f;
            
            /* Non-linear array access pattern */
            int idx = (i * 7) % n;
            arr2[idx] = arr1[i] + arr2[(i-1) % n];
            
            /* Function call to external helper - prevents optimization */
            if (i % 13 == 0) {
                arr1[i] = external_helper(arr1[i]);
            }
        }
        
        /* Cross-iteration dependency */
        farr1[0] = f_acc * 0.01f;
        arr1[0] = acc % 1000;
    }
    
    return acc;
}

/* Another function with different pattern to increase scheduling complexity */
float process_float_data(float* data1, float* data2, int* mask, int n) {
    float result = 0.0f;
    int int_acc = 0;
    
    for (int i = 0; i < n; i++) {
        /* Data-dependent conditional */
        if (mask[i] > 0) {
            data2[i] = data1[i] * data1[(i+1) % n] - data2[(i+n-1) % n];
            
            /* Trigonometric function call */
            result += sinf(data2[i]) * cosf(data1[i]);
        } else {
            data2[i] = data1[i] / (fabsf(data1[(i+2) % n]) + 1.0f);
            result += tanf(data2[i]);
        }
        
        /* Integer operations in float loop */
        int_acc += (int)(data2[i] * 100.0f) ^ mask[i];
        
        /* Periodic complex operation */
        if (i % 7 == 0) {
            data1[i] = logf(fabsf(data2[i]) + 1.0f);
            int_acc = (int_acc << 3) | (int_acc >> 29); /* rotate */
        }
    }
    
    return result + (float)int_acc;
}

/* External helper function definition */
int external_helper(int x) {
    /* Complex enough to not be inlined automatically */
    return (x * 1103515245 + 12345) & 0x7FFFFFFF;
}

int main(int argc, char** argv) {
    /* Use argc to make loop bounds non-constant */
    int n = SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE;
    }
    
    /* Seed RNG with volatile variable */
    srand(g_volatile_seed);
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    int* mask = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX;
        mask[i] = rand() % 2;
    }
    
    /* Process data multiple times */
    int total_acc = 0;
    float total_float = 0.0f;
    
    /* Multiple calls with different parameters */
    for (int rep = 0; rep < 3; rep++) {
        total_acc += process_data(arr1, arr2, farr1, farr2, n, 2);
        total_float += process_float_data(farr1, farr2, mask, n);
        
        /* Modify data between calls */
        for (int i = 0; i < n; i += 3) {
            arr1[i] = (arr1[i] + rep) % 100;
            farr1[i] = farr1[i] * 0.9f + 0.1f;
        }
    }
    
    /* Calculate checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)(farr1[i] * 100) + (int)(farr2[i] * 100);
        checksum = (checksum << 1) | (checksum >> 31); /* rotate */
    }
    
    printf("Result: acc=%d, float=%.6f, checksum=%d\n", 
           total_acc, total_float, checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(mask);
    
    return 0;
}
