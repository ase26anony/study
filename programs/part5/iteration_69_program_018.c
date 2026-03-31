/* test_sched_coverage.c
 * Compile with: gcc -O2 -fsched-verbose=3 test_sched_coverage.c -o test
 * For register pressure: gcc -O3 -fsched-verbose=4 -fsel-sched-pipelining test_sched_coverage.c -o test
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
void high_pressure_loop(float *a, float *b, float *c, int n) {
    /* Many independent variables to create register pressure */
    float t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;
    float t11, t12, t13, t14, t15, t16, t17, t18, t19, t20;
    int i1, i2, i3, i4, i5, i6, i7, i8, i9, i10;
    
    for (int i = 0; i < n; i++) {
        /* Group 1: Independent floating point operations */
        t1 = a[i] * b[i];
        t2 = a[i] + b[i];
        t3 = a[i] - b[i];
        t4 = a[i] / (b[i] + 0.001f);  /* Division for latency */
        
        /* Group 2: More independent operations */
        t5 = t1 * t2;
        t6 = t3 * t4;
        t7 = t1 + t3;
        t8 = t2 + t4;
        
        /* Group 3: Integer operations mixed in */
        i1 = (int)t1;
        i2 = (int)t2;
        i3 = i1 * i2;
        i4 = i1 + i2;
        
        /* Group 4: More FP ops creating pressure */
        t9 = t5 * t6;
        t10 = t7 * t8;
        t11 = t9 + t10;
        t12 = t9 - t10;
        
        /* Use volatile to force dependencies and prevent reordering */
        t13 = t11 * vol_float1;
        t14 = t12 * vol_float2;
        
        /* Group 5: Final computations - many independent results */
        t15 = t13 + t14;
        t16 = t13 - t14;
        t17 = t15 * t16;
        t18 = t15 / (t16 + 0.001f);  /* Another division for latency */
        
        /* Store results creating memory pressure */
        c[i] = t17 + t18;
        
        /* More independent computations to increase candidate count */
        t19 = t17 * t18;
        t20 = t17 - t18;
        
        /* Use results to prevent dead code elimination */
        vol_float1 += t19 * 0.0001f;
        vol_float2 += t20 * 0.0001f;
    }
}

/* Function with artificial dependencies and resource conflicts */
void mixed_dependency(int *arr1, int *arr2, int *arr3, int n) {
    int dep1 = vol_var1;  /* Start with volatile dependency */
    int dep2 = vol_var2;
    
    for (int i = 0; i < n; i++) {
        /* Create a chain of dependencies */
        int temp1 = arr1[i] + dep1;
        int temp2 = arr2[i] + dep2;
        
        /* Independent computations that can be scheduled together */
        int temp3 = arr1[i] * arr2[i];
        int temp4 = arr1[i] - arr2[i];
        int temp5 = arr2[i] - arr1[i];
        int temp6 = arr3[i] * 3;
        int temp7 = arr3[i] / 2;  /* Division for potential delay */
        
        /* More independent groups */
        int temp8 = temp3 + temp4;
        int temp9 = temp5 + temp6;
        int temp10 = temp7 * 2;
        
        /* Create resource conflict: use same operation types */
        int temp11 = temp8 * temp9;  /* Integer multiply */
        int temp12 = temp9 * temp10; /* Another integer multiply - may compete for units */
        int temp13 = temp10 * temp8; /* Third integer multiply */
        
        /* Mix with memory operations */
        arr1[i] = temp11 + temp1;
        arr2[i] = temp12 + temp2;
        arr3[i] = temp13;
        
        /* Update dependencies with volatile for next iteration */
        dep1 = arr1[i] & 0xFF;  /* Create data dependency */
        dep2 = arr2[i] & 0xFF;
        
        /* Inline assembly to clobber registers and force spills */
        __asm__ volatile (
            "movl $0, %%eax\n\t"
            "movl $0, %%ebx\n\t"
            "movl $0, %%ecx\n\t"
            "movl $0, %%edx\n\t"
            "movl $0, %%esi\n\t"
            "movl $0, %%edi\n\t"
            : : : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
        );
    }
}

/* Function with control flow to create priority differences */
void control_flow_priority(float *a, float *b, float *c, int n) {
    float acc1 = 0.0f, acc2 = 0.0f, acc3 = 0.0f;
    float acc4 = 0.0f, acc5 = 0.0f, acc6 = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Critical path operations */
        float x = a[i] * b[i];
        float y = x + c[i];
        
        /* Less critical independent operations */
        float t1 = a[i] + 1.0f;
        float t2 = b[i] - 1.0f;
        float t3 = c[i] * 2.0f;
        float t4 = a[i] / (b[i] + 1.0f);  /* Potential delay */
        
        /* Control flow creates different priorities */
        if (x > 0.5f) {
            /* High priority path */
            acc1 += y * 2.0f;
            acc2 += t1 * t2;
            
            /* More operations in this path */
            float t5 = t3 * t4;
            acc3 += t5;
            
            /* Long latency operation */
            float t6 = 1.0f / (x + 0.001f);  /* Division causes delay */
            acc4 += t6;
        } else {
            /* Alternative path with different operations */
            acc5 += y * 0.5f;
            acc6 += t1 + t2;
            
            /* Different mix of operations */
            float t7 = t3 - t4;
            acc1 += t7 * 0.1f;
        }
        
        /* Independent operations outside control flow */
        float t8 = a[i] * c[i];
        float t9 = b[i] * c[i];
        acc2 += t8 * 0.01f;
        acc3 += t9 * 0.01f;
    }
    
    /* Use results */
    vol_float1 += acc1 + acc2 + acc3;
    vol_float2 += acc4 + acc5 + acc6;
}

/* Main function that creates the hot loops */
int main() {
    /* Allocate and initialize arrays */
    float *fa = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *fb = (float*)malloc(ARRAY_SIZE * sizeof(float));
    float *fc = (float*)malloc(ARRAY_SIZE * sizeof(float));
    int *ia = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *ib = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *ic = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    srand(time(NULL));
    for (int i = 0; i < ARRAY_SIZE; i++) {
        fa[i] = (float)rand() / RAND_MAX;
        fb[i] = (float)rand() / RAND_MAX;
        fc[i] = (float)rand() / RAND_MAX;
        ia[i] = rand() % 100;
        ib[i] = rand() % 100;
        ic[i] = rand() % 100;
    }
    
    /* Perform many iterations to make scheduling analysis relevant */
    for (int iter = 0; iter < ITERATIONS; iter++) {
        /* Call functions with different characteristics */
        high_pressure_loop(fa, fb, fc, ARRAY_SIZE / 4);
        mixed_dependency(ia, ib, ic, ARRAY_SIZE / 4);
        control_flow_priority(fa, fb, fc, ARRAY_SIZE / 4);
        
        /* Modify inputs slightly each iteration */
        for (int i = 0; i < ARRAY_SIZE; i += 8) {
            fa[i] += 0.001f;
            fb[i] -= 0.001f;
            ia[i] = (ia[i] + 1) % 100;
        }
    }
    
    /* Compute checksum to prevent dead code elimination */
    float sum_f = 0.0f;
    int sum_i = 0;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum_f += fa[i] + fb[i] + fc[i];
        sum_i += ia[i] + ib[i] + ic[i];
    }
    
    printf("Result: %f, %d\n", sum_f, sum_i);
    
    free(fa);
    free(fb);
    free(fc);
    free(ia);
    free(ib);
    free(ic);
    
    return 0;
}
