/* ddg_coverage.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with multiple dependency types */
void process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int temp_reg = 0;
    int accum = 0;
    
    /* Outer loop with loop-carried flow dependency */
    for (i = 1; i < n; i++) {
        /* Flow (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i - 1] + b[i];
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* Anti (WAR) dependency: temp_reg read before write */
            c[i] = temp_reg + i;
            temp_reg = b[i] * 2;  /* Write to temp_reg */
            
            /* Output (WAW) dependency on accum */
            accum = a[i] + c[i];
            accum = accum * 3;    /* Second write to accum */
        } else if (i % 3 == 1) {
            /* Different path with register dependencies */
            int local_var = b[i] - a[i];
            
            /* Flow dependency through memory */
            c[i] = local_var + c[i - 1];
            
            /* Anti dependency through memory */
            int temp = a[i];
            a[i] = local_var + temp;
        } else {
            /* Third path with scalar operations */
            accum += b[i];
            
            /* Output dependency in memory */
            c[i] = i * 2;
            c[i] = accum + c[i];  /* Overwrite c[i] */
        }
        
        /* Loop-carried anti dependency across iterations */
        b[i] = accum + temp_reg;
    }
    
    /* Nested loop with reduction */
    for (i = 0; i < n; i++) {
        for (j = 0; j < M; j++) {
            /* Complex addressing with multiple dependencies */
            int idx = (i * M + j) % n;
            
            /* Flow dependency through array */
            a[idx] = a[idx] + b[i] - c[j % n];
            
            /* Register pressure to force spills */
            int r1 = a[idx] * 2;
            int r2 = b[i] + r1;
            int r3 = c[j % n] - r2;
            int r4 = r1 * r3;
            int r5 = r2 + r4;
            
            /* Anti dependency chain */
            temp_reg = r5;
            accum = temp_reg + accum;
            temp_reg = r3;
        }
    }
}

/* Helper with pointer aliasing for additional complexity */
void alias_operations(int *x, int *y, int *z, int n) {
    int i;
    
    /* Potential pointer aliasing creates ambiguous dependencies */
    for (i = 1; i < n - 1; i++) {
        /* Flow dependencies with possible aliasing */
        x[i] = y[i] + z[i];
        y[i + 1] = x[i] * 2;
        z[i - 1] = y[i] + x[i - 1];
        
        /* Output dependency */
        x[i] = x[i] + 1;
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array_a = (int *)malloc(N * sizeof(int));
    int *array_b = (int *)malloc(N * sizeof(int));
    int *array_c = (int *)malloc(N * sizeof(int));
    
    if (!array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-zero values */
    for (int i = 0; i < N; i++) {
        array_a[i] = i;
        array_b[i] = N - i;
        array_c[i] = (i * 3) % 7;
    }
    
    /* Process data to create dependencies */
    process_data(array_a, array_b, array_c, N);
    
    /* Additional processing with aliasing */
    alias_operations(array_a, array_b, array_c, N);
    
    /* Reduction to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array_a[i] + array_b[i] + array_c[i];
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", sum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
