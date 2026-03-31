/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_coverage.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -ftree-vectorize -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>

/* Global variables to prevent optimizations */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];          /* RAW: a[i-1] read, then a[i] written */
        a[i] += a[i-2] + 1;            /* Additional RAW with distance 2 */
        sum += a[i];
    }
    return sum;
}

/* Function with anti and output dependencies (WAR/WAW) */
float test_war_waw_dep(float *x, float *y, int n) {
    float acc = 0.0f;
    float temp;
    
    for (int i = 0; i < n; i++) {
        temp = x[i] + y[i];            /* Read x[i], y[i] */
        x[i] = temp * 2.0f;            /* WAR: x[i] written after being read via temp */
        x[i] = x[i] * 1.5f;            /* WAW: x[i] written again */
        y[i] = temp + x[i];            /* WAR: temp used after x[i] modification */
        acc += x[i] + y[i];
    }
    return acc;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    int *p = ptr1;
    int *q = ptr2;
    
    for (int i = 1; i < n; i++) {
        *p = arr[i] + i;               /* Memory write */
        sum += *q;                     /* Memory read - may alias with p */
        arr[i] = *p + *q;              /* Multiple memory accesses */
        
        /* Alternate pointers to create complex dependencies */
        if (i % 2 == 0) {
            p = &arr[i];
        } else {
            q = &arr[i-1];
        }
    }
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int *data, int *out, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        int val = data[i];
        
        /* Control-dependent operations */
        if (val > threshold) {
            out[i] = val * 2;
            count += val;
        } else if (val < -threshold) {
            out[i] = val / 2;
            count -= val;
        } else {
            out[i] = val + threshold;
            count++;
        }
        
        /* Additional operation with data dependency on control path */
        out[i] += g_volatile;          /* Volatile access creates hard dependency */
    }
    return count;
}

/* Complex nested loop with mixed dependencies */
double test_nested_mixed(double *mat, int rows, int cols) {
    double total = 0.0;
    
    for (int i = 1; i < rows; i++) {
        double row_sum = 0.0;
        
        for (int j = 1; j < cols; j++) {
            /* Multiple dependency types */
            double prev = mat[(i-1)*cols + j];      /* RAW from previous row */
            double left = mat[i*cols + (j-1)];      /* RAW from previous column */
            
            mat[i*cols + j] = prev + left + (i * j); /* Write with dependencies */
            
            /* Anti-dependency */
            double temp = mat[i*cols + j];          /* Read immediately after write */
            mat[i*cols + j] = temp * 0.5;           /* WAR */
            
            row_sum += mat[i*cols + j];
        }
        
        /* Loop-carried output dependency */
        g_array[i % 1024] = (int)row_sum;           /* WAW on global array */
        total += row_sum;
    }
    
    return total;
}

/* Function with volatile and function calls to create memory barriers */
int test_volatile_and_calls(int *arr, int n) {
    static int counter = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Volatile operations create hard dependencies */
        int local_volatile = g_volatile;
        
        /* Output dependency on global */
        g_result = arr[i] + local_volatile;         /* WAW on g_result */
        
        /* Anti-dependency through global */
        sum += g_result;                            /* WAR on g_result */
        
        /* Function call acts as memory clobber */
        counter++;
        if (counter % 100 == 0) {
            /* External memory effect prevents optimization */
            g_volatile = counter;
        }
        
        /* Complex addressing with potential aliasing */
        arr[(i + 1) % n] = sum % 256;
    }
    
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) n = atoi(argv[1]);
    if (n < 10) n = 1000;
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    int *arr3 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    double *dmat = (double*)malloc(n * n * sizeof(double));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 17) % 256;
        arr2[i] = (i * 23) % 256;
        arr3[i] = (i * 31) % 256;
        farr1[i] = (float)(i % 100) * 0.1f;
        farr2[i] = (float)(i % 50) * 0.2f;
    }
    
    for (int i = 0; i < n * n; i++) {
        dmat[i] = (double)(i % 1000) * 0.01;
    }
    
    /* Call all test functions to build various DDG edges */
    int sum1 = test_raw_dep(arr1, arr2, n);
    float sum2 = test_war_waw_dep(farr1, farr2, n);
    int sum3 = test_memory_aliasing(arr3, &arr1[0], &arr2[0], n);
    int sum4 = test_control_dep(arr1, arr2, n, 50);
    double sum5 = test_nested_mixed(dmat, 50, 50);
    int sum6 = test_volatile_and_calls(arr3, n);
    
    /* Aggregate results to prevent dead code elimination */
    int final_result = sum1 + (int)sum2 + sum3 + sum4 + (int)sum5 + sum6;
    
    printf("DDG test result: %d\n", final_result % 1000000);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr1);
    free(farr2);
    free(dmat);
    
    return final_result % 256;
}
