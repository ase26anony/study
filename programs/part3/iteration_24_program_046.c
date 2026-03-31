/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with various dependency patterns */
int compute(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg;
    int scalar = 7;
    
    /* Outer loop with loop-carried flow dependency */
    for (i = 1; i < n; i++) {
        /* Flow (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i] * scalar;
        
        /* Anti (WAR) dependency: b[i] read before modified */
        temp_reg = b[i] + c[i];
        
        /* Output (WAW) dependency on a[i] */
        if (i % 3 == 0) {
            /* This creates a WAW dependency with the earlier a[i] assignment */
            a[i] = temp_reg * 2;
        }
        
        /* Flow dependency on temp_reg */
        c[i] = temp_reg - scalar;
        
        /* Nested inner loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Memory dependencies with array c */
            if (j % 2 == 0) {
                /* Flow dependency through c array */
                c[i] = c[i] + b[j % n];
            } else {
                /* Anti dependency through b array */
                int anti_temp = b[j % n];
                b[j % n] = c[i] * 3;
                c[i] = anti_temp;
            }
            
            /* Register dependencies */
            scalar = scalar + 1;
            temp_reg = scalar * 2;
        }
        
        /* Control flow creates basic block boundaries */
        if (a[i] > 1000) {
            /* More dependencies in this branch */
            b[i] = a[i] / 2;
            scalar = scalar - 5;
        } else {
            /* Different dependencies in else branch */
            c[i] = b[i] * 3;
            scalar = scalar + 2;
        }
        
        /* Accumulate for side effect */
        sum += a[i] + b[i] + c[i];
    }
    
    return sum;
}

/* Another function with different patterns */
void init_arrays(int *a, int *b, int *c, int n) {
    int i;
    for (i = 0; i < n; i++) {
        /* Initialize with some pattern */
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        
        /* Create some initial dependencies */
        if (i > 0) {
            /* Cross-array dependency */
            a[i] = b[i-1] + c[i];
        }
    }
}

int main() {
    int *array1, *array2, *array3;
    int result;
    
    /* Dynamic allocation to avoid stack overflow */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with dependencies */
    init_arrays(array1, array2, array3, N);
    
    /* Main computation with complex dependencies */
    result = compute(array1, array2, array3, N);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional computation to create more opportunities */
    {
        int i;
        int reduction = 0;
        
        /* Another loop with different dependency pattern */
        for (i = N-1; i > 0; i--) {
            /* Reverse loop-carried dependency */
            array1[i-1] = array1[i] + array2[i];
            
            /* Mixed dependencies */
            array2[i] = array3[i] * array1[i-1];
            array3[i] = array2[i] - array1[i];
            
            reduction += array1[i] + array2[i] + array3[i];
        }
        
        printf("Reduction: %d\n", reduction);
    }
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
