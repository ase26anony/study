/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global variables to prevent optimization */
volatile int g_volatile = 0;
int g_array[1024];
int g_result = 0;
int* g_ptr1;
int* g_ptr2;

/* Function 1: Loop with true data dependencies (RAW) and loop-carried dependencies */
int test_raw_dep(int* arr, int n, int step) {
    int sum = 0;
    /* Loop with multiple RAW dependencies and distance > 0 */
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 1 */
        arr[i] = arr[i-1] + g_volatile;
        
        /* Flow dependency with distance 2 (creates longer latency chain) */
        int temp = arr[i-2] * 3;
        
        /* Mixed integer operations with dependency chain */
        sum += arr[i] + temp;
        
        /* Additional dependency to prevent vectorization */
        if (i % step == 0) {
            sum += g_volatile;
        }
    }
    return sum;
}

/* Function 2: Loop with anti (WAR) and output (WAW) dependencies */
int test_war_waw_dep(float* farr, int* iarr, int n) {
    float acc = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Anti-dependency (WAR): read then write to same location */
        float old_val = farr[i];
        farr[i] = iarr[i] * 2.5f + g_volatile;
        acc += old_val;
        
        /* Output dependency (WAW): multiple writes to same location */
        iarr[i] = i * 2;
        iarr[i] = i * 3 + (int)acc;  // Second write creates WAW
        
        /* Introduce register pressure with mixed types */
        if (i % 4 == 0) {
            farr[i] = farr[i] * 1.1f;  // Another write to same location
        }
    }
    return (int)acc;
}

/* Function 3: Loop with memory aliasing through pointers */
int test_memory_aliasing(int* arr1, int* arr2, int n) {
    int sum = 0;
    
    /* Create aliasing pointers */
    int* p = arr1;
    int* q = arr2;
    
    /* Force compiler to assume potential aliasing */
    for (int i = 1; i < n; i++) {
        /* Memory operations that may alias */
        *p = *q + i;
        sum += *p;
        
        /* Pointer arithmetic that creates ambiguous aliasing */
        p = &arr1[i % 3];
        q = &arr2[(i + 1) % 3];
        
        /* Additional memory access with unknown relationship */
        arr1[i] = arr2[n-i] + g_volatile;
    }
    
    return sum;
}

/* Function 4: Loop with control dependencies and complex conditions */
int test_control_dep(int* arr, int n, int threshold) {
    int count = 0;
    
    for (int i = 0; i < n; i++) {
        /* Control-dependent computation */
        int val;
        if (arr[i] > threshold) {
            val = arr[i] * 2;
            count++;
        } else if (arr[i] < -threshold) {
            val = arr[i] / 2;
            count--;
        } else {
            val = arr[i] + g_volatile;
        }
        
        /* Nested control flow */
        if (i % 2 == 0) {
            for (int j = 0; j < 3; j++) {
                val += j * g_volatile;
            }
        }
        
        arr[i] = val;
    }
    
    return count;
}

/* Function 5: Loop with mixed dependencies and function calls */
int test_mixed_deps(int* arr, int n) {
    int total = 0;
    
    for (int i = 1; i < n; i++) {
        /* RAW with floating point */
        float f1 = (float)arr[i-1] * 1.5f;
        
        /* WAR with integer */
        int temp = arr[i];
        arr[i] = (int)f1 + temp;
        
        /* WAW on floating point value */
        f1 = f1 * 2.0f;
        f1 = f1 + 1.0f;  // Another write to f1
        
        /* Memory barrier effect */
        asm volatile("" ::: "memory");
        
        /* Complex expression with multiple uses */
        total += arr[i] + (int)f1 + g_volatile;
        
        /* Loop-carried dependency with distance */
        if (i >= 3) {
            arr[i] += arr[i-3];
        }
    }
    
    return total;
}

/* Function 6: Nested loops with dependencies */
int test_nested_loops(int* matrix, int rows, int cols) {
    int sum = 0;
    
    for (int i = 1; i < rows; i++) {
        for (int j = 1; j < cols; j++) {
            /* Cross-iteration dependencies in both dimensions */
            matrix[i*cols + j] = 
                matrix[(i-1)*cols + j] +    /* Vertical dependency */
                matrix[i*cols + (j-1)] +    /* Horizontal dependency */
                g_volatile;
            
            /* Anti-dependency in inner loop */
            int old = matrix[i*cols + j];
            matrix[i*cols + j] = old * 2;
            sum += old;
            
            /* Control dependency in inner loop */
            if ((i + j) % 5 == 0) {
                matrix[i*cols + j] += 100;
            }
        }
    }
    
    return sum;
}

/* Main function that runs all tests */
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
    int* matrix = (int*)malloc(n * n * sizeof(int));
    
    for (int i = 0; i < n; i++) {
        arr1[i] = i * 3 - n/2;
        arr2[i] = i * 7 % 100;
        farr[i] = (float)i * 0.5f;
    }
    
    for (int i = 0; i < n * n; i++) {
        matrix[i] = i % 97;
    }
    
    g_ptr1 = arr1;
    g_ptr2 = arr2;
    
    /* Run all test functions to create various DDG edges */
    int result = 0;
    
    result += test_raw_dep(arr1, n, 7);
    result += test_war_waw_dep(farr, arr2, n);
    result += test_memory_aliasing(arr1, arr2, n);
    result += test_control_dep(arr1, n, 50);
    result += test_mixed_deps(arr2, n);
    result += test_nested_loops(matrix, 50, 50);
    
    /* Use results to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Store to global to ensure side effects */
    g_result = result;
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(matrix);
    
    return g_result != 0 ? 0 : 1;
}
