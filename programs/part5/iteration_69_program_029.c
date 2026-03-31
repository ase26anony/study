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
    float t1, t2, t3, t4, t5, t6, t7, t8;
    float t9, t10, t11, t12, t13, t14, t15, t16;
    
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
        
        /* Group 3: Cross dependencies to create scheduling decisions */
        t9 = t5 + vol_float1;  /* Volatile dependency creates delay possibility */
        t10 = t6 * vol_float2;
        t11 = t7 - t8;
        t12 = t9 / (t10 + 0.001f);
        
        /* Group 4: Final computations with mixed operations */
        t13 = t11 * t12;
        t14 = t9 + t10;
        t15 = t13 - t14;
        t16 = t15 * 2.0f;
        
        /* Store results with volatile write to prevent elimination */
        c[i] = t16 + (float)vol_var1;
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile ("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", 
                      "r6", "r7", "r8", "r9", "r10", "r11", "r12");
    }
}

/* Function with artificial dependencies and resource conflicts */
__attribute__((noinline))
static void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                            int *restrict arr3, int size) {
    int i;
    int dep1 = vol_var1;  /* Start with volatile dependency */
    int dep2 = vol_var2;
    
    for (i = 0; i < size; i++) {
        /* Long latency chain */
        int temp1 = arr1[i] * dep1;
        int temp2 = arr2[i] + dep2;
        
        /* Artificial resource conflict - mix integer and "memory" ops */
        asm volatile ("# Resource conflict marker" : : : "memory");
        
        int temp3 = temp1 / (vol_var1 + 1);  /* Division has higher latency */
        int temp4 = temp2 - temp3;
        
        /* Volatile read creates scheduling barrier */
        int barrier = vol_var1;
        
        /* Independent instructions that can be scheduled in different orders */
        int indep1 = arr3[i] * 3;
        int indep2 = arr1[i] + 5;
        int indep3 = arr2[i] - 7;
        int indep4 = arr3[i] / 2;
        
        /* Create multiple candidates for scheduler */
        arr1[i] = temp4 + barrier;
        arr2[i] = indep1 + indep2;
        arr3[i] = indep3 * indep4;
        
        /* Update dependencies with volatile */
        dep1 = vol_var2;
        dep2 = arr1[i] & 0xFF;  /* Simple operation to vary priority */
    }
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
static int control_flow_test(int *data, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* Complex condition to create different code paths */
        if (data[i] > 1000) {
            /* Critical path operations */
            sum += data[i] * 3;
            data[i] = sum / (vol_var1 + 1);
            
            /* Inline asm to prevent reordering */
            asm volatile ("# Critical path marker" : : : "memory");
        } 
        else if (data[i] > 500) {
            /* Less critical path */
            sum += data[i] * 2;
            data[i] = sum & 0x7F;
        }
        else {
            /* Cold path - different operations */
            sum += data[i];
            data[i] = ~data[i];
        }
        
        /* Loop-carried dependency */
        sum += vol_var2;  /* Volatile read creates scheduling point */
    }
    
    return sum;
}

/* Main computational kernel */
__attribute__((noinline))
static float compute_kernel(float *a, float *b, float *c, 
                           int *int_arr1, int *int_arr2, int *int_arr3,
                           int size) {
    float total = 0.0f;
    int i;
    
    for (i = 0; i < ITERATIONS; i++) {
        /* Alternate between different computation patterns */
        if (i % 3 == 0) {
            high_pressure_loop(a, b, c, size / 4);
        } 
        else if (i % 3 == 1) {
            mixed_dependency(int_arr1, int_arr2, int_arr3, size / 8);
        }
        else {
            control_flow_test(int_arr1, size / 16);
        }
        
        /* Cross-iteration dependency to prevent parallelization */
        vol_var1 = (vol_var1 * 1103515245 + 12345) & 0x7FFFFFFF;
        vol_var2 = vol_var1 % 100;
        
        /* Accumulate results to prevent dead code elimination */
        total += a[i % size] + b[i % size] + c[i % size] +
                 int_arr1[i % size] + int_arr2[i % size] + int_arr3[i % size];
    }
    
    return total;
}

int main(void) {
    /* Allocate and initialize data */
    float *array_a = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *array_b = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *array_c = (float*)malloc(ARRAY_SIZE * sizeof(float));
    
    int *int_array1 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_array2 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *int_array3 = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array_a[i] = (float)rand() / RAND_MAX * 100.0f;
        array_b[i] = (float)rand() / RAND_MAX * 100.0f;
        array_c[i] = 0.0f;
        
        int_array1[i] = rand() % 2000;
        int_array2[i] = rand() % 2000;
        int_array3[i] = rand() % 2000;
    }
    
    printf("Starting scheduling test computation...\n");
    
    /* Perform the computation that should trigger scheduling debug output */
    float result = compute_kernel(array_a, array_b, array_c,
                                  int_array1, int_array2, int_array3,
                                  ARRAY_SIZE);
    
    printf("Computation complete. Result checksum: %f\n", result);
    printf("Sample values: a[0]=%f, int1[0]=%d\n", array_a[0], int_array1[0]);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(int_array1);
    free(int_array2);
    free(int_array3);
    
    return 0;
}
