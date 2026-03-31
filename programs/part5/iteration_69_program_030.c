/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test_sched_coverage
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test_sched_pressure
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1024
#define ITERS 100000

/* Volatile variables to prevent optimization and create dependencies */
volatile int vol_a = 1, vol_b = 2, vol_c = 3;
volatile float vol_f1 = 1.0f, vol_f2 = 2.0f, vol_f3 = 3.0f;

/* Function to create high register pressure with independent instructions */
__attribute__((noinline))
void high_pressure_loop(float *restrict a, float *restrict b, float *restrict c, 
                        float *restrict d, int n) {
    int i;
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    
    for (i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);  /* Division for latency */
        
        /* Group 2: More independent operations */
        t5 = c[i] * d[i];
        t6 = c[i] + d[i];
        t7 = c[i] - d[i];
        t8 = c[i] / (d[i] + 0.001f);
        
        /* Group 3: Cross dependencies to create scheduling decisions */
        t9 = t1 + t5;
        t10 = t2 * t6;
        t11 = t3 - t7;
        t12 = t4 / (t8 + 0.001f);
        
        /* Group 4: More operations to increase pressure */
        t13 = t9 * vol_f1;
        t14 = t10 + vol_f2;
        t15 = t11 - vol_f3;
        t16 = t12 / vol_f1;
        
        /* Group 5: Final computations with artificial dependencies */
        t17 = t13 + t14;
        t18 = t15 * t16;
        t19 = t17 - t18;
        t20 = t19 / (t17 + 0.001f);
        
        /* Store results back, creating memory dependencies */
        a[i] = t17 + t20;
        b[i] = t18 - t20;
        c[i] = t19 * t20;
        d[i] = t20;
    }
}

/* Function with mixed dependencies and resource conflicts */
__attribute__((noinline))
void mixed_dependency(int *restrict arr1, int *restrict arr2, 
                      float *restrict farr1, float *restrict farr2, int n) {
    int i;
    int tmp1, tmp2, tmp3, tmp4, tmp5;
    float ftmp1, ftmp2, ftmp3, ftmp4;
    
    /* Artificial dependency chain with volatile */
    tmp1 = vol_a;
    for (i = 0; i < n; i++) {
        /* Integer operations with dependencies */
        tmp2 = arr1[i] + tmp1;
        tmp3 = arr2[i] * tmp2;
        tmp4 = tmp3 - vol_b;
        tmp5 = tmp4 / (vol_c + 1);
        
        /* Floating point operations competing for FP units */
        ftmp1 = farr1[i] * 1.2345f;
        ftmp2 = farr2[i] / 2.3456f;  /* Division for latency */
        ftmp3 = ftmp1 + ftmp2;
        ftmp4 = ftmp3 - (float)tmp5;
        
        /* Mix operations to create resource conflicts */
        arr1[i] = tmp5 + (int)ftmp4;
        farr2[i] = ftmp4 * (float)arr2[i];
        
        /* Volatile read creates memory barrier effect */
        tmp1 = vol_a + i % 16;
    }
    
    /* Inline assembly to clobber registers and force spills */
    asm volatile (
        "mov $0, %%eax\n\t"
        "mov $0, %%ebx\n\t"
        "mov $0, %%ecx\n\t"
        "mov $0, %%edx\n\t"
        "mov $0, %%esi\n\t"
        "mov $0, %%edi\n\t"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
}

/* Function with control flow to create priority differences */
__attribute__((noinline))
float control_flow_priority(float *arr, int n) {
    float sum = 0.0f;
    float prod = 1.0f;
    float max_val = -1e30f;
    float min_val = 1e30f;
    int i;
    
    for (i = 0; i < n; i++) {
        /* Conditional creates different instruction priorities */
        if (arr[i] > 0.5f) {
            sum += arr[i] * 1.5f;
            prod *= (arr[i] + 0.1f);
            
            /* Nested condition for more complexity */
            if (arr[i] > max_val) {
                max_val = arr[i];
                /* Volatile access inside condition */
                max_val += (float)vol_a * 0.01f;
            }
        } else {
            sum -= arr[i] * 0.5f;
            prod /= (arr[i] + 1.1f);
            
            if (arr[i] < min_val) {
                min_val = arr[i];
                min_val -= (float)vol_b * 0.01f;
            }
        }
        
        /* Loop-carried dependency */
        arr[i] = sum * 0.01f + prod * 0.01f + max_val + min_val;
    }
    
    return sum + prod + max_val + min_val;
}

/* Main computational kernel */
__attribute__((noinline))
float compute_kernel(void) {
    static float arr1[SIZE], arr2[SIZE], arr3[SIZE], arr4[SIZE];
    static int iarr1[SIZE], iarr2[SIZE];
    int i;
    float result = 0.0f;
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (float)(i % 100) * 0.01f;
        arr2[i] = (float)((i + 37) % 100) * 0.015f;
        arr3[i] = (float)((i * 3) % 100) * 0.02f;
        arr4[i] = (float)((i * 7) % 100) * 0.025f;
        iarr1[i] = i * 2;
        iarr2[i] = i * 3 + 1;
    }
    
    /* Perform multiple passes with different characteristics */
    for (i = 0; i < ITERS / 100; i++) {
        /* High register pressure section */
        high_pressure_loop(arr1, arr2, arr3, arr4, SIZE / 4);
        
        /* Mixed dependency section */
        mixed_dependency(iarr1, iarr2, arr1, arr2, SIZE / 4);
        
        /* Control flow priority section */
        result += control_flow_priority(arr3, SIZE / 4);
        
        /* Modify volatile to change dependencies */
        if (i % 100 == 0) {
            vol_a = (vol_a * 3 + 1) % 100;
            vol_b = (vol_b * 5 + 1) % 100;
            vol_f1 = sinf((float)i * 0.01f);
        }
    }
    
    return result;
}

int main(void) {
    float final_result;
    clock_t start, end;
    
    srand(time(NULL));
    
    printf("Starting scheduling test...\n");
    start = clock();
    
    final_result = compute_kernel();
    
    end = clock();
    
    printf("Result: %f\n", final_result);
    printf("Time: %f seconds\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    /* Use result to prevent dead code elimination */
    if (final_result > 1000000.0f) {
        printf("Result is large\n");
    }
    
    return 0;
}
