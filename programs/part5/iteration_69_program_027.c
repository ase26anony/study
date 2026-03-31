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
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, float *restrict result) {
    /* Many independent calculations to create scheduling candidates */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    /* Unrolled loop with many temporaries - high register pressure */
    for (int i = 0; i < ARRAY_SIZE; i += 8) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i] + vol_float1;
        t2 = a[i+1] / b[i+1] - vol_float2;  /* Division has longer latency */
        t3 = c[i] * d[i] * t1;
        t4 = c[i+1] / d[i+1] / t2;
        
        /* Group 2: More independent operations */
        t5 = t1 * t2 + t3;
        t6 = t2 / t3 - t4;  /* Another division */
        t7 = t3 * t4 * vol_float1;
        t8 = t4 / vol_float2 + t1;
        
        /* Group 3: Mix with integer operations */
        t9 = (float)((int)t5 + (int)t6 + vol_var1);
        t10 = (float)((int)t7 * (int)t8 - vol_var2);
        
        /* Group 4: More operations creating scheduling candidates */
        t11 = t9 * t10;
        t12 = t10 / t9;  /* Division */
        t13 = sqrtf(fabsf(t11));  /* Function call-like operation */
        t14 = t12 * t13;
        
        /* Group 5: Final computations */
        t15 = t11 + t12 + t13 + t14;
        t16 = t11 * t12 * t13 * t14;
        t17 = t15 / t16;  /* Division */
        t18 = t16 - t15;
        
        /* Store results with memory dependencies */
        result[i] = t17 + t18;
        result[i+1] = t17 - t18;
        
        /* More independent groups for the next elements */
        t19 = a[i+2] * b[i+2] * c[i+2];
        t20 = a[i+3] / b[i+3] / c[i+3];  /* Division */
        
        /* Inline assembly to clobber registers and force spills */
        asm volatile("" : : : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
                     "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15");
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr1, float *restrict farr2) {
    int x1, x2, x3, x4, x5, x6, x7, x8;
    float f1, f2, f3, f4, f5, f6, f7, f8;
    
    /* Create artificial dependencies with volatile variables */
    x1 = vol_var1;
    x2 = vol_var2;
    
    /* Long latency chain */
    f1 = farr1[0] / farr2[0];  /* FP division - long latency */
    x3 = arr1[0] + x1;         /* Integer add - can execute in parallel */
    f2 = f1 * 3.14159f;        /* Depends on f1 */
    x4 = arr2[0] * x2;         /* Independent integer */
    
    /* Resource conflict: multiple divides */
    f3 = farr1[1] / farr2[1];  /* Another division - competes for divider unit */
    f4 = farr1[2] / farr2[2];  /* Yet another division */
    
    /* Independent integer operations that can be scheduled together */
    x5 = arr1[1] + arr2[1];
    x6 = arr1[2] - arr2[2];
    x7 = arr1[3] * arr2[3];
    x8 = arr1[4] / (arr2[4] + 1);  /* Integer division */
    
    /* More FP operations mixing with integer results */
    f5 = (float)x5 + f3;
    f6 = (float)x6 * f4;
    f7 = f5 / f6;  /* Another division */
    f8 = sqrtf(fabsf(f7));
    
    /* Store results to prevent elimination */
    arr1[0] = x3 + x4 + x5 + x6;
    farr1[0] = f8;
    
    /* Memory barrier to create scheduling boundaries */
    asm volatile("" : : : "memory");
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
int control_flow_priority(int *data, int size) {
    int sum = 0;
    int product = 1;
    int threshold = size / 2;
    
    for (int i = 0; i < size; i++) {
        /* Critical path operations */
        if (data[i] > threshold) {
            sum += data[i] * vol_var1;  /* Depends on volatile */
            product *= (data[i] + 1);
        } else {
            sum -= data[i] / (vol_var2 + 1);  /* Integer division */
            product /= (data[i] > 0 ? data[i] : 1);
        }
        
        /* Independent computations that can be scheduled in parallel */
        int temp1 = data[i] * i;
        int temp2 = data[i] + i;
        int temp3 = temp1 - temp2;
        int temp4 = temp1 * temp2;
        
        /* Mix with floating point */
        float ftemp = (float)temp3 / (float)(temp4 + 1);
        sum += (int)(ftemp * 100.0f);
    }
    
    return sum + product;
}

/* Main function that creates the hot loop for scheduler analysis */
int main() {
    /* Initialize data */
    float *a = malloc(ARRAY_SIZE * sizeof(float));
    float *b = malloc(ARRAY_SIZE * sizeof(float));
    float *c = malloc(ARRAY_SIZE * sizeof(float));
    float *d = malloc(ARRAY_SIZE * sizeof(float));
    float *result = malloc(ARRAY_SIZE * sizeof(float));
    
    int *int_data1 = malloc(ARRAY_SIZE * sizeof(int));
    int *int_data2 = malloc(ARRAY_SIZE * sizeof(int));
    float *float_data1 = malloc(ARRAY_SIZE * sizeof(float));
    float *float_data2 = malloc(ARRAY_SIZE * sizeof(float));
    
    srand(time(NULL));
    
    for (int i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)rand() / RAND_MAX * 100.0f;
        b[i] = (float)rand() / RAND_MAX * 100.0f + 0.1f;  /* Avoid divide by zero */
        c[i] = (float)rand() / RAND_MAX * 100.0f;
        d[i] = (float)rand() / RAND_MAX * 100.0f + 0.1f;
        
        int_data1[i] = rand() % 1000;
        int_data2[i] = rand() % 1000 + 1;
        float_data1[i] = (float)rand() / RAND_MAX * 100.0f;
        float_data2[i] = (float)rand() / RAND_MAX * 100.0f + 0.1f;
    }
    
    /* Hot loop that will be scheduled */
    int final_sum = 0;
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Modify volatile variables to create dependencies */
        vol_var1 = (iter % 256) + 1;
        vol_var2 = ((iter * 17) % 256) + 1;
        vol_float1 = (float)(iter % 100) * 0.1f + 0.5f;
        vol_float2 = (float)((iter * 23) % 100) * 0.1f + 0.5f;
        
        /* Call functions with different scheduling characteristics */
        high_pressure_loop(a, b, c, d, result);
        mixed_dependency(int_data1, int_data2, float_data1, float_data2);
        final_sum += control_flow_priority(int_data1, ARRAY_SIZE / 4);
        
        /* Prevent loop unrolling from simplifying too much */
        if (iter % 100 == 0) {
            asm volatile("" : : : "memory");
        }
    }
    
    /* Use results to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum += result[i] + int_data1[i] + float_data1[i];
    }
    
    printf("Final sum: %d, Checksum: %f\n", final_sum, checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(d); free(result);
    free(int_data1); free(int_data2);
    free(float_data1); free(float_data2);
    
    return 0;
}
