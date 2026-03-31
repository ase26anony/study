/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERS 100000

/* Volatile variables to prevent optimizations and create dependencies */
volatile int vol_var1, vol_var2, vol_var3;
volatile double vol_double1, vol_double2;

/* Inline assembly to clobber registers and create pressure */
#define CLOBBER_MANY_REGS() \
    __asm__ volatile ("" : : : \
        "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
        "r8", "r9", "r10", "r11", "r12", \
        "d0", "d1", "d2", "d3", "d4", "d5", "d6", "d7", \
        "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15", \
        "memory")

/* Function to create high register pressure with independent instructions */
void high_pressure_loop(int *restrict a, int *restrict b, int *restrict c, 
                        int *restrict d, int n) {
    /* Many independent integer operations to create scheduling candidates */
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent integer operations */
        int t1 = a[i] * 3;
        int t2 = b[i] + 7;
        int t3 = c[i] - 5;
        int t4 = d[i] ^ 0xFF;
        
        /* Group 2: More independent operations */
        int t5 = t1 << 2;
        int t6 = t2 >> 1;
        int t7 = t3 & 0x7F;
        int t8 = t4 | 0x80;
        
        /* Group 3: Cross dependencies to create priority differences */
        int t9 = t5 + t6;
        int t10 = t7 * t8;
        int t11 = t9 - t10;
        int t12 = t11 ^ t5;
        
        /* Store results creating register pressure */
        a[i] = t9;
        b[i] = t10;
        c[i] = t11;
        d[i] = t12;
        
        /* Artificial dependency through volatile */
        vol_var1 = t9;
        CLOBBER_MANY_REGS();
    }
}

/* Function with mixed operation types and resource conflicts */
void mixed_dependency(float *restrict f1, float *restrict f2, 
                      double *restrict d1, double *restrict d2, int n) {
    /* Create long latency floating point operations */
    for (int i = 0; i < n; i++) {
        /* Long latency divide operations */
        double dval1 = d1[i] / 3.14159;
        double dval2 = d2[i] / 2.71828;
        
        /* Float operations competing for different units */
        float fval1 = f1[i] * 1.5f;
        float fval2 = f2[i] * 2.5f;
        
        /* More operations to create scheduling candidates */
        double dval3 = dval1 * dval2;
        float fval3 = fval1 + fval2;
        
        /* Volatile access creates memory barrier and delays */
        vol_double1 = dval3;
        vol_var2 = (int)fval3;
        
        /* Complex expression with dependencies */
        d1[i] = dval3 + (double)fval3;
        d2[i] = dval1 - dval2;
        f1[i] = fval3 * 0.5f;
        f2[i] = fval1 - fval2;
        
        /* Inline assembly with specific constraints to create conflicts */
        __asm__ volatile (
            "fdiv %%d0, %%d1, %%d2\n\t"
            "fmul %%s3, %%s4, %%s5\n\t"
            : 
            : 
            : "d0", "d1", "d2", "s3", "s4", "s5", "memory"
        );
    }
}

/* Function with control flow to create priority variations */
void control_flow_variations(int *arr, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    for (int i = 0; i < n; i++) {
        /* Independent accumulations - scheduler can reorder these */
        sum1 += arr[i];
        sum2 += arr[i] * 2;
        sum3 += arr[i] * 3;
        sum4 += arr[i] * 4;
        
        /* Conditional creates different execution paths */
        if (arr[i] > 1000) {
            /* Critical path operations */
            sum1 -= 100;
            sum2 += 50;
            CLOBBER_MANY_REGS();
        } else if (arr[i] < 100) {
            /* Less critical path */
            sum3 *= 2;
            sum4 /= 2;
        }
        
        /* Loop with break creates additional control flow */
        for (int j = 0; j < 4; j++) {
            if (j == arr[i] % 4) {
                sum1 ^= j;
                break;
            }
            sum2 += j;
        }
        
        vol_var3 = sum1;
    }
    
    /* Prevent dead code elimination */
    arr[0] = sum1 + sum2 + sum3 + sum4;
}

/* Main computational kernel that combines all patterns */
void compute_kernel(int *int_data1, int *int_data2, 
                    float *float_data1, float *float_data2,
                    double *double_data1, double *double_data2,
                    int size) {
    /* Multiple passes to give scheduler many opportunities */
    for (int pass = 0; pass < 3; pass++) {
        high_pressure_loop(int_data1, int_data2, 
                          int_data1 + size/2, int_data2 + size/2, 
                          size/4);
        
        mixed_dependency(float_data1, float_data2,
                        double_data1, double_data2,
                        size/2);
        
        control_flow_variations(int_data1, size);
        
        /* Additional independent computation blocks */
        for (int i = 0; i < size/8; i++) {
            /* Many independent statements that can be scheduled in any order */
            int idx = i * 8;
            int_data1[idx] = int_data1[idx] * int_data2[idx];
            int_data1[idx+1] = int_data1[idx+1] + int_data2[idx+1];
            int_data1[idx+2] = int_data1[idx+2] - int_data2[idx+2];
            int_data1[idx+3] = int_data1[idx+3] ^ int_data2[idx+3];
            int_data1[idx+4] = int_data1[idx+4] | int_data2[idx+4];
            int_data1[idx+5] = int_data1[idx+5] & int_data2[idx+5];
            int_data1[idx+6] = int_data1[idx+6] << 2;
            int_data1[idx+7] = int_data1[idx+7] >> 1;
            
            float_data1[i] = sqrtf(float_data1[i] * float_data2[i]);
            double_data1[i] = pow(double_data1[i], 1.01);
        }
    }
}

int main() {
    /* Allocate and initialize data */
    int *int_data1 = malloc(SIZE * sizeof(int));
    int *int_data2 = malloc(SIZE * sizeof(int));
    float *float_data1 = malloc(SIZE * sizeof(float));
    float *float_data2 = malloc(SIZE * sizeof(float));
    double *double_data1 = malloc(SIZE * sizeof(double));
    double *double_data2 = malloc(SIZE * sizeof(double));
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        int_data1[i] = rand() % 1000;
        int_data2[i] = rand() % 1000;
        float_data1[i] = (float)rand() / RAND_MAX * 100.0f;
        float_data2[i] = (float)rand() / RAND_MAX * 100.0f;
        double_data1[i] = (double)rand() / RAND_MAX * 1000.0;
        double_data2[i] = (double)rand() / RAND_MAX * 1000.0;
    }
    
    /* Warm up and run the kernel multiple times */
    clock_t start = clock();
    
    for (int iter = 0; iter < ITERS; iter++) {
        compute_kernel(int_data1, int_data2,
                      float_data1, float_data2,
                      double_data1, double_data2,
                      SIZE);
        
        /* Prevent loop invariant code motion */
        vol_var1 = iter;
        vol_double1 = (double)iter;
    }
    
    clock_t end = clock();
    
    /* Compute checksum to prevent dead code elimination */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        checksum += int_data1[i] + int_data2[i];
        checksum += (long long)(float_data1[i] * 1000);
        checksum += (long long)(double_data1[i] * 1000);
    }
    
    printf("Checksum: %lld\n", checksum);
    printf("Time: %.2f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Cleanup */
    free(int_data1);
    free(int_data2);
    free(float_data1);
    free(float_data2);
    free(double_data1);
    free(double_data2);
    
    return 0;
}
