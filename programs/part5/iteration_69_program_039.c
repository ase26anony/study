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
volatile int vol_var1 = 1;
volatile int vol_var2 = 2;
volatile float vol_float1 = 1.5f;
volatile float vol_float2 = 2.5f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
static void high_pressure_loop(float *restrict a, float *restrict b, 
                               float *restrict c, int size) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < size; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);
        
        /* Group 2: More independent operations */
        t5 = t1 * t2;
        t6 = t3 + t4;
        t7 = t1 - t4;
        t8 = t2 / (t3 + 0.001f);
        
        /* Group 3: Creating data dependencies */
        t9 = t5 + t6;
        t10 = t7 * t8;
        t11 = t9 - t10;
        t12 = t5 / (t6 + 0.001f);
        
        /* Group 4: More operations with mixed dependencies */
        t13 = t11 * t12;
        t14 = t9 + t10;
        t15 = t11 - t12;
        t16 = t13 / (t14 + 0.001f);
        
        /* Group 5: Final computations */
        t17 = t13 * t14;
        t18 = t15 + t16;
        t19 = t17 - t18;
        t20 = t13 / (t15 + 0.001f);
        
        /* Store results with artificial dependencies on volatile */
        c[i] = t19 * t20 * vol_float1 + vol_float2;
    }
}

/* Function with mixed operation types and artificial delays */
__attribute__((noinline))
static void mixed_dependency_ops(int *restrict arr1, int *restrict arr2,
                                 float *restrict farr1, float *restrict farr2,
                                 int size) {
    int i;
    
    for (i = 0; i < size; i++) {
        /* Integer operations with dependencies */
        int x1 = arr1[i] + vol_var1;
        int x2 = arr2[i] - vol_var2;
        int x3 = x1 * x2;
        int x4 = x1 / (x2 + 1);
        
        /* Floating point operations (potential long latency) */
        float f1 = farr1[i] * 1.234567f;
        float f2 = farr2[i] / 3.141592f;  /* Division has longer latency */
        float f3 = f1 + f2;
        float f4 = f1 - f2;
        
        /* Memory operations with potential aliasing */
        arr1[i] = x3 + (int)f3;
        arr2[i] = x4 + (int)f4;
        
        /* More operations to create scheduling choices */
        farr1[i] = f3 * vol_float1;
        farr2[i] = f4 / (vol_float2 + 0.001f);
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile (
            "movl $0, %%eax\n\t"
            "movl $1, %%ebx\n\t"
            "movl $2, %%ecx\n\t"
            "movl $3, %%edx\n\t"
            "movl $4, %%esi\n\t"
            "movl $5, %%edi\n\t"
            : /* no outputs */
            : /* no inputs */
            : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with control flow to create priority variations */
__attribute__((noinline))
static int control_flow_priority(int *arr, int size) {
    int i, sum = 0;
    int temp1, temp2, temp3, temp4, temp5;
    
    for (i = 0; i < size; i++) {
        /* Independent operations that can be scheduled in different orders */
        temp1 = arr[i] * 2;
        temp2 = arr[i] + 1;
        temp3 = arr[i] - 1;
        temp4 = arr[i] / 2;
        temp5 = arr[i] % 3;
        
        /* Control flow creates different priority paths */
        if (arr[i] > 0) {
            /* Critical path operations */
            sum += temp1 * temp2;
            sum -= temp3;
        } else {
            /* Less critical path */
            sum += temp4 + temp5;
        }
        
        /* More independent operations */
        temp1 = sum * 3;
        temp2 = sum + 5;
        temp3 = sum - 2;
        
        /* Nested conditionals for more complex control flow */
        if (sum % 2 == 0) {
            temp4 = temp1 * temp2;
        } else {
            temp4 = temp2 / (temp3 + 1);
        }
        
        sum = temp4;
    }
    
    return sum;
}

/* Main computational kernel */
__attribute__((noinline))
static float compute_kernel(float *a, float *b, float *c, 
                           int *arr1, int *arr2,
                           float *farr1, float *farr2,
                           int size) {
    float result = 0.0f;
    int i;
    
    /* Create scheduling opportunities with many independent statements */
    for (i = 0; i < size; i += 4) {
        /* Unrolled loop creates many independent instructions */
        float r1 = a[i] * b[i] + c[i];
        float r2 = a[i+1] * b[i+1] - c[i+1];
        float r3 = a[i+2] / (b[i+2] + 0.001f) * c[i+2];
        float r4 = a[i+3] + b[i+3] / (c[i+3] + 0.001f);
        
        /* Independent integer operations */
        int i1 = arr1[i] * arr2[i];
        int i2 = arr1[i+1] + arr2[i+1];
        int i3 = arr1[i+2] - arr2[i+2];
        int i4 = arr1[i+3] / (arr2[i+3] + 1);
        
        /* Mix results */
        result += r1 * i1 + r2 * i2 - r3 * i3 + r4 * i4;
        
        /* More operations to increase register pressure */
        farr1[i] = r1 * 1.1f;
        farr2[i] = r2 * 0.9f;
        farr1[i+1] = r3 / 1.1f;
        farr2[i+1] = r4 * 1.2f;
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize data */
    float *a = malloc(ARRAY_SIZE * sizeof(float));
    float *b = malloc(ARRAY_SIZE * sizeof(float));
    float *c = malloc(ARRAY_SIZE * sizeof(float));
    float *farr1 = malloc(ARRAY_SIZE * sizeof(float));
    float *farr2 = malloc(ARRAY_SIZE * sizeof(float));
    int *arr1 = malloc(ARRAY_SIZE * sizeof(int));
    int *arr2 = malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f;
        c[i] = (float)rand() / RAND_MAX * 100.0f;
        farr1[i] = (float)rand() / RAND_MAX * 50.0f;
        farr2[i] = (float)rand() / RAND_MAX * 50.0f;
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    float total_result = 0.0f;
    int control_result = 0;
    
    /* Perform multiple iterations to ensure scheduler sees hot code */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions that create different scheduling scenarios */
        high_pressure_loop(a, b, c, ARRAY_SIZE);
        mixed_dependency_ops(arr1, arr2, farr1, farr2, ARRAY_SIZE);
        control_result += control_flow_priority(arr1, ARRAY_SIZE);
        total_result += compute_kernel(a, b, c, arr1, arr2, farr1, farr2, ARRAY_SIZE);
        
        /* Modify data slightly each iteration to prevent complete optimization */
        a[iter % ARRAY_SIZE] += 0.1f;
        b[iter % ARRAY_SIZE] -= 0.1f;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f, Control: %d\n", total_result, control_result);
    
    /* Cleanup */
    free(a); free(b); free(c);
    free(farr1); free(farr2);
    free(arr1); free(arr2);
    
    return 0;
}
