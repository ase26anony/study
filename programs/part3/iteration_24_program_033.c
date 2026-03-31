/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Function with complex data dependencies */
int process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg = 0;
    int scalar = 7;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            c[i] = temp_reg * 2;
            temp_reg = scalar + i;
            
            /* OUTPUT (WAW) dependency on a[i] */
            a[i] = c[i] + 1;
        } else if (i % 3 == 1) {
            /* Mixed memory and register dependencies */
            int t1 = b[i] * 3;
            int t2 = a[i] + t1;
            
            /* Another OUTPUT (WAW) dependency */
            a[i] = t2 - scalar;
            
            /* FLOW dependency through memory */
            c[i] = a[i] / 2;
        } else {
            /* Complex dependency chain */
            int x = b[i];
            int y = x * x;
            
            /* ANTI dependency on c[i] */
            scalar = c[i] + y;
            
            /* FLOW dependency through scalar */
            a[i] = scalar - x;
        }
        
        /* Loop-carried dependency with distance > 1 */
        if (i > 2) {
            /* Distance-2 flow dependency */
            b[i] = a[i-2] + c[i-1];
        }
        
        /* Reduction to prevent elimination */
        sum += a[i] + b[i] + c[i];
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* Cross-iteration dependencies in inner loop */
            if (j > 0) {
                /* Flow dependency in j dimension */
                c[j] = c[j-1] + i;
                
                /* Anti dependency */
                int old_val = b[j];
                b[j] = a[i] + j;
                a[i] = old_val * 2;
            }
            
            /* Output dependency in inner loop */
            if (j % 4 == 0) {
                c[j] = i * j;
                c[j] = c[j] + 5;  /* Another output on same location */
            }
            
            sum += b[j] - c[j];
        }
    }
    
    return sum;
}

/* Another function with pointer aliasing for additional complexity */
int process_with_aliasing(int *arr1, int *arr2, int n) {
    int i;
    int result = 0;
    
    /* Potential aliasing creates ambiguous dependencies */
    for (i = 1; i < n; i++) {
        /* These may alias, creating conservative dependencies */
        arr1[i] = arr2[i-1] + 1;
        arr2[i] = arr1[i] * 2;
        
        /* Self-dependency */
        arr1[i] = arr1[i] + arr1[i-1];
        
        result += arr1[i] + arr2[i];
    }
    
    return result;
}

int main() {
    int *array1, *array2, *array3;
    int i, total = 0;
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = i * 2;
        array3[i] = i * 3;
    }
    
    /* Process data with complex dependencies */
    total += process_data(array1, array2, array3, N);
    
    /* Process with potential aliasing */
    total += process_with_aliasing(array1, array2, N/2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
