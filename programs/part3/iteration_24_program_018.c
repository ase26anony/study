/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex loop with various dependency patterns */
void process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int temp_reg = 0;
    int accum = 0;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Conditional to create basic block boundaries */
        if (i % 2 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            c[i] = temp_reg + i;
            temp_reg = a[i] * 2;  /* Write to temp_reg */
            
            /* OUTPUT (WAW) dependency on accum */
            accum = c[i] + b[i];
            accum = accum * 3;    /* Second write to accum */
        } else {
            /* Different path with its own dependencies */
            int local_var = b[i] - a[i];
            
            /* FLOW dependency within else block */
            c[i] = local_var * 2;
            accum += c[i];        /* Read-modify-write on accum */
            
            /* Another OUTPUT dependency */
            local_var = i * i;    /* Overwrite local_var */
        }
        
        /* Cross-iteration FLOW dependency through array */
        b[i] = b[i-1] + 1;
    }
    
    /* Nested loop with register and memory dependencies */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* Complex memory access pattern */
            int idx = (i * M + j) % n;
            
            /* Multiple FLOW dependencies */
            int val1 = a[idx] + c[j];
            int val2 = b[idx] * val1;
            
            /* ANTI dependency: read a[idx] before writing */
            a[idx] = val2 - val1;
            
            /* OUTPUT dependency on c array */
            c[j] = val1 + val2;
            c[j] = c[j] * 2;      /* Second write to c[j] */
        }
    }
    
    /* Use results to prevent elimination */
    printf("Result: %d\n", accum + a[n-1] + b[n-1]);
}

/* Helper with pointer aliasing to create ambiguous dependencies */
void process_with_aliasing(int *x, int *y, int n) {
    int i;
    
    /* Potential aliasing creates conservative dependencies */
    for (i = 1; i < n; i++) {
        /* These may alias, creating memory dependencies */
        x[i] = y[i-1] + 1;
        y[i] = x[i-1] * 2;
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = N - i;
        array3[i] = i * 2;
    }
    
    /* Process data with complex dependencies */
    process_data(array1, array2, array3, N);
    
    /* Additional processing with aliasing */
    process_with_aliasing(array1, array2, N/2);
    
    /* Final reduction to ensure computation isn't eliminated */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array1[i] + array2[i] + array3[i];
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
