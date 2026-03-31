#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100

/* Prevent optimization and create dependencies */
volatile int volatile_var = 0;

/* Functions to prevent optimization */
__attribute__((noinline)) 
int use_value(int val) {
    volatile_var = val;
    return val + 1;
}

__attribute__((noinline))
void modify_array(int* arr, int idx) {
    arr[idx] = arr[idx] * 2 + 1;
}

/* Test 1: Flow (RAW) dependency - cumulative sum */
__attribute__((noinline))
int test_flow_dependency(int* arr) {
    int sum = 0;
    /* Flow dependency: sum from previous iteration used in current */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  // RAW: read arr[i], write sum
        arr[i] = sum;   // Potential anti-dependency on sum
    }
    return sum;
}

/* Test 2: Anti (WAR) dependency */
__attribute__((noinline))
int test_anti_dependency(int* arr) {
    int temp = 0;
    /* Anti-dependency: read then write same location */
    for (int i = 0; i < SIZE - 1; i++) {
        temp = arr[i];          // Read arr[i]
        arr[i] = arr[i + 1];    // Write arr[i] (WAR on temp's source)
        arr[i + 1] = temp;      // Write arr[i+1]
    }
    return arr[SIZE - 1];
}

/* Test 3: Output (WAW) dependency */
__attribute__((noinline))
int test_output_dependency(int* arr) {
    /* Output dependency: multiple writes to same location */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;             // First write
        arr[i] = arr[i] * 2;    // Second write (WAW)
        if (i % 2 == 0) {
            arr[i] = arr[i] + 1; // Third write with control flow
        }
    }
    return arr[SIZE / 2];
}

/* Test 4: Mixed dependencies with control flow */
__attribute__((noinline))
int test_mixed_dependencies(int* arr1, int* arr2) {
    int sum = 0;
    /* Complex pattern with flow, anti, and control dependencies */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency through sum */
        sum = arr1[i - 1] + sum;
        
        /* Anti-dependency with conditional */
        if (i % 3 == 0) {
            int temp = arr2[i];
            arr2[i] = sum;
            sum = temp;  // Flow dependency on sum
        } else if (i % 3 == 1) {
            /* Output dependency */
            arr1[i] = i;
            arr1[i] = arr1[i] * 3;
        } else {
            /* Simple flow */
            arr2[i] = arr2[i - 1] + 1;
        }
    }
    return sum;
}

/* Test 5: Nested loops with outer-loop dependencies */
__attribute__((noinline))
int test_nested_dependencies(int matrix[][N]) {
    int total = 0;
    /* Outer loop dependency carried to inner loop */
    for (int i = 1; i < N; i++) {
        for (int j = 0; j < N; j++) {
            /* Flow dependency on previous row */
            matrix[i][j] = matrix[i - 1][j] + matrix[i][j];
            total += matrix[i][j];
        }
        /* Anti-dependency within outer loop */
        int temp = matrix[i][0];
        matrix[i][0] = total;
        total = temp % 100;
    }
    return total;
}

/* Test 6: Different data types for varied DDG edges */
__attribute__((noinline))
float test_float_dependencies(float* farr, double* darr) {
    float fsum = 0.0f;
    double dsum = 0.0;
    
    /* Mixed type dependencies */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with type conversion */
        fsum = farr[i - 1] + fsum;
        dsum = darr[i - 1] + dsum;
        
        /* Anti-dependency between different types */
        float ftemp = farr[i];
        farr[i] = (float)dsum;
        darr[i] = (double)fsum;
        fsum = ftemp;
    }
    return fsum + (float)dsum;
}

/* Test 7: Pointer aliasing to prevent optimization */
__attribute__((noinline))
int test_pointer_aliasing(int* arr1, int* arr2) {
    /* arr1 and arr2 may alias */
    int result = 0;
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with potential aliasing */
        arr1[i] = arr1[i - 1] + arr2[i];
        
        /* Anti-dependency that might alias */
        arr2[i - 1] = arr1[i] * 2;
        
        result += arr1[i];
    }
    return result;
}

/* Test 8: Complex loop with multiple carried dependencies */
__attribute__((noinline))
int test_complex_carried_deps(int* arr) {
    int x = 0, y = 0, z = 0;
    
    /* Multiple interleaved carried dependencies */
    for (int i = 0; i < SIZE; i++) {
        x = arr[i] + x;      // Flow dep on x
        y = x + y;           // Flow dep on y, also on x
        z = y + z;           // Flow dep on z, also on y
        
        if (i % 4 == 0) {
            arr[i] = z;      // Output dep on arr[i]
        } else if (i % 4 == 1) {
            int temp = arr[i];
            arr[i] = x;
            x = temp;        // Anti-dep through x
        } else if (i % 4 == 2) {
            arr[i] = arr[i] + y;  // Flow dep on arr[i]
        } else {
            arr[i] = z - y;  // Flow deps on z and y
        }
    }
    return x + y + z;
}

int main() {
    /* Initialize data */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int matrix[N][N];
    
    srand(time(NULL));
    
    /* Initialize arrays with random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        farr[i] = (float)(rand() % 100) / 10.0f;
        darr[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 50;
        }
    }
    
    /* Run all tests to trigger DDG construction */
    int result = 0;
    
    result += test_flow_dependency(arr1);
    result += test_anti_dependency(arr2);
    result += test_output_dependency(arr1);
    result += test_mixed_dependencies(arr1, arr2);
    result += test_nested_dependencies(matrix);
    
    float float_result = test_float_dependencies(farr, darr);
    result += (int)float_result;
    
    result += test_pointer_aliasing(arr1, arr2);
    result += test_complex_carried_deps(arr1);
    
    /* Use results to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Volatile var: %d\n", volatile_var);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    
    return result % 100;
}
