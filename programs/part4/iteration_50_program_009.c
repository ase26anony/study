/* test_ddg_coverage.c
 * 
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler, specifically targeting the create_ddg_edge()
 * function in ddg.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;

/* Function prototypes */
int test_raw_dep(int *a, int *b, int n);
int test_war_waw_dep(int *a, int *b, int *c, int n);
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n);
int test_control_dep(int *data, int *mask, int n);
int test_mixed_deps(float *fa, int *ia, double *da, int n);
int test_nested_loops(int *matrix, int rows, int cols);
int test_loop_carried_dep(int *a, int n, int distance);

/* Test 1: True Data Dependencies (RAW - Read After Write) */
int test_raw_dep(int *a, int *b, int n) {
    int sum = 0;
    
    /* Multiple RAW dependencies with different distances */
    for (int i = 2; i < n; i++) {
        /* Flow dependency chain: a[i] depends on a[i-1] and a[i-2] */
        a[i] = a[i-1] + b[i] + g_volatile;      /* distance 1 */
        a[i] += a[i-2] * 2;                     /* distance 2 */
        sum += a[i];
    }
    
    /* Another RAW pattern with floating point */
    float temp = 0.0f;
    for (int i = 1; i < n; i++) {
        temp = temp * 1.1f + a[i];              /* accumulator dependency */
        sum += (int)temp;
    }
    
    return sum;
}

/* Test 2: Anti and Output Dependencies (WAR/WAW) */
int test_war_waw_dep(int *a, int *b, int *c, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int t1 = a[i] + b[i];                   /* Read a[i], b[i] */
        a[i] = t1 * 2;                          /* WAR: Write a[i] after reading it */
        
        /* WAW: Multiple writes to same location */
        c[i] = i * 3;
        c[i] = c[i] + t1;                       /* Output dependency */
        
        /* More complex WAR */
        b[i] = a[i] + c[i];                     /* Reads a[i] written above */
        a[i] = b[i] / 2;                        /* WAR on a[i] again */
        
        sum += a[i] + b[i] + c[i];
    }
    
    return sum;
}

/* Test 3: Memory Aliasing Dependencies */
int test_memory_aliasing(int *arr1, int *arr2, int *ptr1, int *ptr2, int n) {
    int sum = 0;
    
    /* Force potential aliasing */
    ptr1 = &arr1[n/2];
    ptr2 = &arr2[0];
    
    for (int i = 1; i < n-1; i++) {
        /* Memory operations that may alias */
        arr1[i] = arr2[i-1] + g_volatile;
        *ptr1 = arr1[i] * 2;                    /* May alias with arr1 */
        
        /* Pointer arithmetic creating ambiguous dependencies */
        *(ptr2 + i % 10) = arr1[i] + i;
        arr2[i] = *(ptr1 - i % 5) + arr2[i];
        
        sum += arr1[i] + arr2[i];
    }
    
    return sum;
}

/* Test 4: Control Dependencies */
int test_control_dep(int *data, int *mask, int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (mask[i] > 0) {
            data[i] = data[i] * 2 + 1;
            sum += data[i];
        } else if (mask[i] < 0) {
            data[i] = data[i] / 2 - 1;
            sum -= data[i];
        } else {
            data[i] = g_volatile;
            sum += i;
        }
        
        /* Nested control flow */
        for (int j = 0; j < (i % 8); j++) {
            data[i] += j * mask[i];
        }
    }
    
    return sum;
}

/* Test 5: Mixed Data Types and Dependencies */
int test_mixed_deps(float *fa, int *ia, double *da, int n) {
    float fsum = 0.0f;
    int isum = 0;
    double dsum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Mixed type dependencies */
        fa[i] = fa[i-1] * 1.5f + ia[i];         /* Float with int input */
        ia[i] = (int)fa[i] * 2 + i;             /* Int with float input */
        da[i] = da[i-1] + fa[i] + ia[i];        /* Double dependency chain */
        
        /* Cross-type anti-dependency */
        float ftemp = fa[i];
        fa[i] = (float)da[i] * 0.5f;            /* WAR on fa[i] */
        isum += (int)ftemp;
        
        fsum += fa[i];
        dsum += da[i];
    }
    
    return isum + (int)fsum + (int)dsum;
}

/* Test 6: Nested Loops with Complex Dependencies */
int test_nested_loops(int *matrix, int rows, int cols) {
    int sum = 0;
    
    /* Outer loop with carried dependency */
    for (int i = 1; i < rows; i++) {
        int row_start = i * cols;
        
        /* Inner loop with multiple dependency types */
        for (int j = 1; j < cols; j++) {
            /* RAW: Depends on previous row and column */
            int idx = row_start + j;
            matrix[idx] = matrix[idx - 1] +                /* Same row, prev col */
                         matrix[idx - cols] +              /* Prev row, same col */
                         matrix[idx - cols - 1] +          /* Prev row, prev col */
                         g_volatile;
            
            /* WAR: Update and reuse */
            int temp = matrix[idx];
            matrix[idx] = temp * (i + j);
            
            sum += matrix[idx];
        }
        
        /* Control dependency in outer loop */
        if (i % 3 == 0) {
            for (int j = 0; j < cols; j += 2) {
                matrix[row_start + j] += sum % 100;
            }
        }
    }
    
    return sum;
}

/* Test 7: Explicit Loop-Carried Dependencies with Distance */
int test_loop_carried_dep(int *a, int n, int distance) {
    int sum = 0;
    
    /* Distance > 1 loop-carried dependency */
    for (int i = distance; i < n; i++) {
        a[i] = a[i - distance] * 3 - a[i - 1] + i;  /* Multiple distances */
        sum += a[i];
    }
    
    /* Another pattern with varying distance */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;
        int idx2 = (i * 3) % n;
        if (idx != idx2 && idx < n && idx2 < n) {
            a[idx] = a[idx2] + g_volatile;          /* Non-uniform access pattern */
        }
        sum += a[i % n];
    }
    
    return sum;
}

/* Main function that runs all tests */
int main(int argc, char *argv[]) {
    const int N = 1024;
    const int ROWS = 64;
    const int COLS = 16;
    
    /* Allocate and initialize test data */
    int *data1 = (int*)malloc(N * sizeof(int));
    int *data2 = (int*)malloc(N * sizeof(int));
    int *data3 = (int*)malloc(N * sizeof(int));
    int *mask = (int*)malloc(N * sizeof(int));
    float *fdata = (float*)malloc(N * sizeof(float));
    double *ddata = (double*)malloc(N * sizeof(double));
    int *matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pseudo-random values */
    srand(time(NULL));
    for (int i = 0; i < N; i++) {
        data1[i] = rand() % 100;
        data2[i] = rand() % 100;
        data3[i] = rand() % 100;
        mask[i] = (rand() % 3) - 1;  /* Values: -1, 0, 1 */
        fdata[i] = (float)(rand() % 100) / 10.0f;
        ddata[i] = (double)(rand() % 100) / 5.0;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = rand() % 100;
    }
    
    /* Update volatile to prevent compile-time computation */
    g_volatile = rand() % 10;
    
    /* Run all tests to create various DDG edges */
    int result = 0;
    
    result += test_raw_dep(data1, data2, N);
    result += test_war_waw_dep(data1, data2, data3, N);
    result += test_memory_aliasing(data1, data2, &data1[N/2], &data2[0], N);
    result += test_control_dep(data1, mask, N);
    result += test_mixed_deps(fdata, data2, ddata, N);
    result += test_nested_loops(matrix, ROWS, COLS);
    result += test_loop_carried_dep(data3, N, 3);
    
    /* Store to global to prevent dead code elimination */
    g_result = result;
    
    /* Print checksum */
    printf("DDG Test Checksum: %d\n", result);
    printf("Volatile value used: %d\n", g_volatile);
    
    /* Cleanup */
    free(data1);
    free(data2);
    free(data3);
    free(mask);
    free(fdata);
    free(ddata);
    free(matrix);
    
    return 0;
}
