/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with multiple dependency patterns */
int complex_dependency_pattern(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg;
    int scalar = 5;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* ANTI (WAR) dependency: temp_reg reads a[i] before it's overwritten */
        temp_reg = a[i];
        
        /* OUTPUT (WAW) dependency: a[i] written twice */
        a[i] = temp_reg * scalar;
        
        /* Another FLOW dependency with register */
        scalar = scalar + 1;
        
        /* Control flow to create basic block boundaries */
        if (i % 3 == 0) {
            /* FLOW dependency across basic blocks */
            c[i] = a[i] * 2;
            
            /* ANTI dependency with array */
            temp_reg = c[i];
            c[i] = b[i] + scalar;
            
            /* Use temp_reg to prevent elimination */
            sum += temp_reg;
        } else if (i % 3 == 1) {
            /* Different dependency pattern */
            b[i] = a[i] + c[i-1];
            
            /* OUTPUT dependency on scalar */
            scalar = b[i] % 7;
        } else {
            /* Third pattern with output dependency */
            a[i] = b[i] * c[i];
            
            /* Register dependency chain */
            temp_reg = scalar;
            scalar = temp_reg + i;
        }
        
        /* Loop-carried dependency with distance > 1 */
        if (i > 2) {
            a[i] += a[i-2];
        }
    }
    
    return sum;
}

/* Nested loops for more complex DDG */
void nested_loop_pattern(int *arr1, int *arr2, int *arr3, int n, int m) {
    int i, j;
    int acc = 0;
    
    for (i = 0; i < n; i++) {
        /* Initialize with anti dependency */
        int local = arr1[i];
        arr1[i] = i;
        
        for (j = 1; j < m; j++) {
            /* Cross-iteration dependency in inner loop */
            arr2[j] = arr2[j-1] + arr1[i];
            
            /* Multiple dependencies in one statement */
            arr3[i*m + j] = arr2[j] * arr3[i*m + j-1] + local;
            
            /* Register pressure */
            local = local + arr3[i*m + j] % 11;
        }
        
        /* Output dependency after inner loop */
        arr1[i] = local;
        
        /* Conditional with dependencies */
        if (arr1[i] > 100) {
            acc += arr1[i];
            arr2[0] = acc;  /* Anti dependency on arr2[0] from next iteration */
        }
    }
    
    /* Prevent dead code elimination */
    arr3[0] = acc;
}

/* Main function with multiple dependency patterns */
int main() {
    /* Allocate arrays on heap to avoid stack overflow */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * M * sizeof(int));
    int *array4 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3 || !array4) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        array4[i] = i * 2;
    }
    
    for (int i = 0; i < N * M; i++) {
        array3[i] = i % 17;
    }
    
    /* Call functions with complex dependencies */
    int result1 = complex_dependency_pattern(array1, array2, array4, N);
    
    nested_loop_pattern(array1, array2, array3, N/4, M);
    
    /* Final computation with dependencies to prevent elimination */
    int final_sum = 0;
    for (int i = 1; i < N; i++) {
        /* Loop-carried flow dependency */
        array1[i] = array1[i-1] + array2[i];
        
        /* Anti dependency */
        int temp = array1[i];
        array1[i] = array4[i] * 3;
        
        /* Use temp to prevent elimination */
        final_sum += temp;
        
        /* Output dependency on scalar */
        static int counter = 0;
        counter = counter + 1;
    }
    
    printf("Result1: %d, Final sum: %d\n", result1, final_sum);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
