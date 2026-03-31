/* test_ddg_edges.c
 * Program designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -funroll-loops -fmodulo-sched -c test_ddg_edges.c
 * Or: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves -c test_ddg_edges.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Global variables to prevent optimizations */
volatile int g_volatile_counter = 0;
int g_global_array[1024];
float g_float_array[1024];
int* g_ptr1;
int* g_ptr2;

/* Function to create true data dependencies (RAW) */
int test_raw_dep(int* arr, int n) {
    int sum = 0;
    /* Loop with flow dependencies across iterations */
    for (int i = 2; i < n; i++) {
        /* True dependency: arr[i] depends on arr[i-1] and arr[i-2] */
        arr[i] = arr[i-1] + arr[i-2] + i;
        /* Another flow dependency chain */
        sum += arr[i];
    }
    return sum;
}

/* Function to create anti and output dependencies (WAR/WAW) */
float test_war_waw_dep(float* farr, int* iarr, int n) {
    float temp = 0.0f;
    float x = 1.5f;
    
    for (int i = 0; i < n; i++) {
        /* Anti-dependency (WAR): farr[i] is read, then written */
        float old_val = farr[i];  /* Read */
        farr[i] = old_val * x + i; /* Write - creates WAR with above read */
        
        /* Output dependency (WAW): iarr[i] written twice */
        iarr[i] = i * 2;          /* First write */
        iarr[i] = iarr[i] + 1;    /* Second write - creates WAW */
        
        /* Mix with control flow to complicate DDG */
        if (i % 3 == 0) {
            temp += farr[i];
        } else {
            temp -= farr[i];
        }
        
        /* Volatile access to prevent reordering */
        g_volatile_counter++;
    }
    return temp;
}

/* Function with memory aliasing dependencies */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    int result = 0;
    int* p = arr1;
    int* q = arr2;
    
    /* Force potential aliasing */
    if (n > 100) {
        q = arr1 + 10;  /* q now potentially aliases with p */
    }
    
    for (int i = 1; i < n - 1; i++) {
        /* Memory dependencies with potential aliasing */
        *p = *p + i;
        *q = *q * 2;
        
        /* Pointer arithmetic - changes what p/q point to */
        p = &arr1[i];
        q = &arr2[(i * 7) % n];
        
        /* Complex addressing with array indices */
        arr1[(i * 3) % n] = arr2[(i * 5) % n] + arr1[i];
        
        result += *p + *q;
    }
    
    return result;
}

/* Function with control dependencies */
double test_control_dep(double* darr, int n, int threshold) {
    double sum = 0.0;
    int counter = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computations */
        if (darr[i] > threshold) {
            darr[i] = sqrt(darr[i]);
            counter += 2;
        } else if (darr[i] < -threshold) {
            darr[i] = darr[i] * darr[i];
            counter -= 1;
        } else {
            darr[i] = sin(darr[i]);
            counter++;
        }
        
        /* Loop-carried dependency with distance > 1 */
        if (i >= 3) {
            darr[i] += darr[i-3] * 0.5;
        }
        
        sum += darr[i];
        
        /* Function call acts as memory clobber */
        g_volatile_counter = (g_volatile_counter * 13 + 17) % 1000;
    }
    
    return sum + counter;
}

/* Nested loop with mixed dependencies */
int test_nested_loops(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 1; i < rows - 1; i++) {
        for (int j = 1; j < cols - 1; j++) {
            /* Stencil computation with multiple dependencies */
            matrix[i][j] = (matrix[i-1][j] + matrix[i+1][j] +
                          matrix[i][j-1] + matrix[i][j+1]) / 4;
            
            /* Anti-dependency within inner loop */
            int temp = matrix[i][j];
            matrix[i][j] = temp + (i * j);
            
            /* Output dependency */
            g_global_array[j] = i;
            g_global_array[j] = g_global_array[j] * 2;
            
            total += matrix[i][j];
        }
        
        /* Loop-carried dependency across outer loop iterations */
        if (i > 0) {
            matrix[i][0] += matrix[i-1][cols-1];
        }
    }
    
    return total;
}

/* Function with floating-point and integer mixed operations */
float test_mixed_types(float* farr, int* iarr, int n) {
    float fsum = 0.0f;
    int isum = 0;
    
    for (int i = 0; i < n; i++) {
        /* Mixed type operations */
        float fval = farr[i];
        int ival = iarr[i];
        
        /* Type conversions create specific dependencies */
        farr[i] = fval + (float)ival;
        iarr[i] = ival + (int)fval;
        
        /* Different data types for DDG edges */
        if (fval > 0) {
            fsum += farr[i] * 2.0f;
        } else {
            isum += iarr[i] * 3;
        }
        
        /* Memory dependency with different types */
        g_float_array[i % 1024] = fsum;
        g_global_array[i % 1024] = isum;
    }
    
    return fsum + (float)isum;
}

/* Main function that calls all test cases */
int main(int argc, char** argv) {
    int n = 1000;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize data */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr = (float*)malloc(n * sizeof(float));
    double* darr = (double*)malloc(n * sizeof(double));
    
    /* Initialize matrix for nested loops */
    int rows = 100, cols = 100;
    int** matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (i * cols + j) % 100;
        }
    }
    
    /* Initialize arrays with pseudo-random data */
    for (int i = 0; i < n; i++) {
        arr1[i] = (i * 13 + 7) % 100;
        arr2[i] = (i * 17 + 11) % 100;
        farr[i] = (float)((i * 19 + 13) % 100) / 3.0f;
        darr[i] = (double)((i * 23 + 17) % 200 - 100) / 5.0;
    }
    
    int result = 0;
    float fresult = 0.0f;
    double dresult = 0.0;
    
    /* Call all test functions to create various DDG edges */
    result += test_raw_dep(arr1, n);
    fresult += test_war_waw_dep(farr, arr2, n);
    result += test_memory_aliasing(arr1, arr2, n);
    dresult += test_control_dep(darr, n, 10);
    result += test_nested_loops(matrix, rows, cols);
    fresult += test_mixed_types(farr, arr2, n);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %.2f, %.2f\n", result, fresult, dresult);
    printf("Volatile counter: %d\n", g_volatile_counter);
    
    /* Cleanup */
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return 0;
}
