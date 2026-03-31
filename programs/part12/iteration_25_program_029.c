/* test_ddg_coverage.c
 * Program to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -ftree-vectorize -funroll-loops -fmodulo-sched -fdump-tree-dd test_ddg_coverage.c -o test_ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITERATIONS 1000000

/* Prevent inlining to maintain dependencies */
__attribute__((noinline)) 
void init_array(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (i * 3 + 7) % 100;
    }
}

__attribute__((noinline))
int test_flow_dependency(int *arr, int size) {
    volatile int sum = 0;  /* volatile prevents optimization */
    
    /* Strong flow dependency: sum depends on previous iteration */
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* RAW: read arr[i], write sum */
        arr[i] = sum;   /* WAW: write arr[i] (also creates output dep) */
    }
    
    return sum;
}

__attribute__((noinline))
int test_anti_dependency(int *arr, int size) {
    int temp[SIZE];
    
    /* Anti-dependency pattern: read then write */
    for (int i = 0; i < size - 1; i++) {
        temp[i] = arr[i];      /* Read arr[i] */
        arr[i] = arr[i + 1];   /* Write arr[i] - WAR with previous read */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += temp[i];
    }
    return sum;
}

__attribute__((noinline))
int test_output_dependency(int *arr, int size) {
    /* Multiple writes to same location (output dependency) */
    for (int i = 0; i < size; i++) {
        arr[i] = i;           /* First write */
        arr[i] = arr[i] * 2;  /* Second write - WAW dependency */
        arr[i] = arr[i] + 1;  /* Third write - WAW dependency */
    }
    
    /* Compute checksum */
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

__attribute__((noinline))
int test_nested_dependency(int arr[][16], int rows, int cols) {
    int sum = 0;
    
    /* Nested loops with cross-iteration dependencies */
    for (int i = 1; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Flow dependency on previous row */
            arr[i][j] = arr[i-1][j] + arr[i][j];
            sum += arr[i][j];
        }
    }
    
    return sum;
}

__attribute__((noinline))
int test_mixed_types_dependency(void) {
    /* Mixed data types to trigger different DDG edge types */
    float farr[SIZE];
    double darr[SIZE];
    int iarr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        farr[i] = i * 1.5f;
        darr[i] = i * 2.5;
        iarr[i] = i;
    }
    
    /* Complex dependency chain with mixed types */
    volatile float fsum = 0.0f;
    volatile double dsum = 0.0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with type conversion */
        farr[i] = farr[i-1] * 1.1f + iarr[i];
        
        /* Anti-dependency */
        double temp = darr[i];      /* Read */
        darr[i] = temp * 0.9;       /* Write - WAR */
        
        /* Output dependency */
        iarr[i] = i * 2;            /* Write 1 */
        iarr[i] = iarr[i] + 1;      /* Write 2 - WAW */
        
        fsum += farr[i];
        dsum += darr[i];
    }
    
    return (int)(fsum + dsum);
}

__attribute__((noinline))
int test_control_flow_dependency(int *arr, int size) {
    int sum = 0;
    
    /* Control flow creates complex dependency patterns */
    for (int i = 1; i < size; i++) {
        if (i % 3 == 0) {
            /* Flow dependency path */
            arr[i] = arr[i-1] + arr[i];
            sum += arr[i];
        } else if (i % 3 == 1) {
            /* Anti-dependency path */
            int temp = arr[i];
            arr[i] = temp * 2;
            sum -= arr[i];
        } else {
            /* Output dependency path */
            arr[i] = i;
            arr[i] = arr[i] * 3;
            sum += arr[i];
        }
    }
    
    return sum;
}

__attribute__((noinline))
int test_pointer_aliasing_dependency(int *a, int *b, int size) {
    /* Pointer aliasing creates ambiguous dependencies */
    int sum = 0;
    
    for (int i = 1; i < size; i++) {
        /* Compiler can't tell if a and b alias */
        a[i] = b[i-1] + 1;   /* Possible flow dep if a == b */
        b[i] = a[i] * 2;     /* Possible anti-dep if a == b */
        sum += a[i] + b[i];
    }
    
    return sum;
}

__attribute__((noinline))
int test_reduction_dependency(double *arr, int size) {
    /* Reduction with carried dependency */
    double sum = 0.0;
    
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Strong flow dependency on sum */
        arr[i] = sum;   /* Output dependency on arr[i] */
    }
    
    return (int)sum;
}

int main(void) {
    int array1[SIZE];
    int array2[SIZE];
    int matrix[64][16];
    double darray[SIZE];
    
    /* Initialize data */
    init_array(array1, SIZE);
    init_array(array2, SIZE);
    
    for (int i = 0; i < 64; i++) {
        for (int j = 0; j < 16; j++) {
            matrix[i][j] = (i * 16 + j) % 100;
        }
    }
    
    for (int i = 0; i < SIZE; i++) {
        darray[i] = i * 1.234;
    }
    
    /* Run all tests to trigger various DDG edge creations */
    int total = 0;
    
    total += test_flow_dependency(array1, SIZE);
    total += test_anti_dependency(array2, SIZE);
    total += test_output_dependency(array1, SIZE);
    total += test_nested_dependency(matrix, 64, 16);
    total += test_mixed_types_dependency();
    total += test_control_flow_dependency(array2, SIZE);
    total += test_pointer_aliasing_dependency(array1, array2, SIZE);
    total += test_reduction_dependency(darray, SIZE);
    
    /* Additional iterations to increase optimization opportunities */
    for (int iter = 0; iter < ITERATIONS / 1000; iter++) {
        total += test_flow_dependency(array1, 100);
        total += test_anti_dependency(array2, 100);
    }
    
    printf("Total checksum: %d\n", total);
    printf("DDG edge creation should be triggered during compilation with:\n");
    printf("  gcc -O3 -ftree-vectorize -funroll-loops -fmodulo-sched -fdump-tree-dd test_ddg_coverage.c\n");
    
    return total != 0 ? 0 : 1;
}
