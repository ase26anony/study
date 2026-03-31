/* test_ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's scheduler
 * Compile with: gcc -O2 -fmodulo-sched -funroll-loops -c test_ddg_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        a[i] = a[i-1] + b[i];        /* RAW: a[i-1] read, then a[i] written */
        a[i] += a[i-2] + 1;          /* Additional RAW with distance 2 */
        sum += a[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
int test_war_waw_dep(float *fa, float *fb, int n) {
    float temp = 0.0f;
    /* Mix of WAR and WAW dependencies */
    for (int i = 0; i < n; i++) {
        float t1 = fa[i] + fb[i];    /* Read fa[i], fb[i] */
        fa[i] = t1 * 2.0f;           /* WAR: fa[i] written after being read above */
        
        /* WAW chain */
        fb[i] = fa[i] + 1.0f;        /* First write to fb[i] */
        fb[i] = fb[i] * 3.0f;        /* WAW: Second write to fb[i] */
        
        temp += fa[i] + fb[i];
    }
    return (int)temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    /* arr1 and arr2 may alias if pointers overlap */
    for (int i = 1; i < n; i++) {
        arr1[i] = arr2[i-1] + g_volatile;  /* Potential memory dependency */
        *ptr1 = arr1[i] * 2;                /* Pointer access - may alias with arr1 */
        *ptr2 = *ptr1 + i;                  /* Another pointer - may alias */
        sum += arr1[i] + *ptr1;
    }
    return sum;
}

/* Function with control dependencies */
int test_control_dep(int *data, int *mask, int n) {
    int count = 0;
    /* Loop with internal branching creating control dependencies */
    for (int i = 0; i < n; i++) {
        if (mask[i] > 0) {                  /* Control dependency */
            data[i] = data[i] * 2 + 1;
            if (i % 3 == 0) {               /* Nested control dependency */
                data[i] += g_volatile;
            }
        } else {
            data[i] = data[i] / 2;
        }
        
        /* Additional operation with loop-carried dependency */
        if (i > 0) {
            data[i] += data[i-1] & 0xFF;    /* RAW with control */
        }
        count += data[i];
    }
    return count;
}

/* Complex nested loop with mixed dependencies */
int test_nested_loops(int *mat, int rows, int cols) {
    int total = 0;
    /* Outer loop with carried dependency */
    for (int r = 1; r < rows; r++) {
        /* Inner loop with multiple dependency types */
        for (int c = 1; c < cols; c++) {
            int idx = r * cols + c;
            int prev_idx = (r-1) * cols + c;
            int left_idx = r * cols + (c-1);
            
            /* RAW: Flow dependency from previous row and column */
            mat[idx] = mat[prev_idx] + mat[left_idx];
            
            /* WAR: Anti-dependency chain */
            int temp = mat[idx];
            mat[idx] = temp * 2 - mat[prev_idx];
            
            /* Memory dependency with potential aliasing */
            g_array[idx % 1024] = mat[idx] + g_volatile;
            
            total += mat[idx];
        }
    }
    return total;
}

/* Function with volatile and function calls to create memory barriers */
int test_volatile_and_calls(int *arr, int n) {
    int result = 0;
    /* Function calls create memory clobbering dependencies */
    for (int i = 0; i < n; i++) {
        /* Volatile access creates hard dependency */
        int v = g_volatile;
        
        /* Mixed integer/float operations */
        float f = (float)arr[i] / (v + 1);
        arr[i] = (int)(f * 100.0f);
        
        /* Output dependency chain (WAW) */
        arr[i] = arr[i] + i;
        arr[i] = arr[i] * 2;
        arr[i] = arr[i] - i;
        
        /* Another volatile access */
        g_volatile = i & 0xFF;
        
        result += arr[i];
    }
    return result;
}

/* Main function that runs all tests */
int main(int argc, char **argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    int *mask = (int*)malloc(n * sizeof(int));
    
    int rows = 50, cols = 50;
    int *matrix = (int*)malloc(rows * cols * sizeof(int));
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 37) % 100;
        arr2[i] = (i * 53) % 100;
        farr1[i] = (float)(i % 50);
        farr2[i] = (float)((i * 7) % 50);
        mask[i] = i % 3;
    }
    
    for (int i = 0; i < rows * cols; i++) {
        matrix[i] = i % 100;
    }
    
    /* Create potentially aliasing pointers */
    int *ptr1 = &arr1[n/2];
    int *ptr2 = &arr2[n/3];
    
    /* Run all test functions to create various DDG edges */
    int checksum = 0;
    
    checksum += test_raw_dep(arr1, arr2, n);
    checksum += test_war_waw_dep(farr1, farr2, n);
    checksum += test_memory_aliasing(arr1, arr2, ptr1, ptr2, n/2);
    checksum += test_control_dep(arr1, mask, n);
    checksum += test_nested_loops(matrix, rows, cols);
    checksum += test_volatile_and_calls(arr2, n);
    
    /* Use the results to prevent dead code elimination */
    g_result = checksum;
    
    /* Print checksum to ensure computation isn't optimized away */
    printf("Result checksum: %d\n", checksum % 1000);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(mask);
    free(matrix);
    
    return checksum % 256;
}
