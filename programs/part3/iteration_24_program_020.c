/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with various dependency patterns */
int process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg;
    int scalar = 5;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* ANTI (WAR) dependency: temp_reg reads a[i] before it's overwritten */
            temp_reg = a[i] * 2;
            
            /* OUTPUT (WAW) dependency: a[i] written twice */
            a[i] = temp_reg + scalar;
            
            /* Another FLOW dependency within the same iteration */
            c[i] = a[i] + i;
        } else if (i % 3 == 1) {
            /* Different path with register dependencies */
            scalar = scalar + 1;  /* FLOW on scalar */
            a[i] = b[i] * scalar; /* FLOW on scalar and b[i] */
            
            /* Complex expression with multiple dependencies */
            temp_reg = a[i] + c[i-1]; /* FLOW on a[i] and c[i-1] (loop-carried) */
            c[i] = temp_reg / 2;
        } else {
            /* Third path with output dependencies */
            int local_var = a[i] + 10;
            
            /* OUTPUT (WAW) on a[i] */
            a[i] = local_var;
            a[i] = a[i] * 3;  /* Another OUTPUT on a[i] */
            
            /* ANTI (WAR) on local_var */
            c[i] = local_var + b[i];
            local_var = i;  /* Overwrites local_var */
        }
        
        /* Cross-iteration FLOW dependency (distance > 0) */
        if (i > 10) {
            /* Depends on value from 5 iterations ago */
            b[i] = b[i-5] + 1;
        }
        
        /* Reduction operation to prevent elimination */
        sum += a[i] + c[i];
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* 2D array-like access patterns */
            int idx = i * M + j;
            
            /* Multiple interleaved dependencies */
            int old_val = a[idx % n];
            a[idx % n] = b[j] + old_val;  /* FLOW on old_val, ANTI on b[j] */
            b[j] = old_val - a[(idx-1) % n]; /* FLOW on old_val and a[] */
            
            /* Register pressure */
            scalar = scalar * 2 + 1;
        }
        sum += scalar;
    }
    
    return sum;
}

/* Helper with pointer aliasing to create memory dependencies */
void alias_dependencies(int *x, int *y, int *z, int n) {
    int i;
    
    /* Assume x and y might alias */
    for (i = 1; i < n; i++) {
        /* Potential FLOW dependency if x and y alias */
        x[i] = y[i-1] + z[i];
        y[i] = x[i] * 2;
        
        /* Output dependency */
        z[i] = i;
        z[i] = z[i] + x[i];  /* FLOW on z[i] */
    }
}

int main() {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(N * sizeof(int));
    int *array_b = (int*)malloc(N * sizeof(int));
    int *array_c = (int*)malloc(N * sizeof(int));
    
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
    int result = process_data(array_a, array_b, array_c, N);
    
    /* Additional processing with aliasing */
    alias_dependencies(array_a, array_b, array_c, N/2);
    
    /* Final computation to use results */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array_a[i] + array_b[i] + array_c[i];
    }
    
    /* Print to prevent dead code elimination */
    printf("Result: %d, Final sum: %d\n", result, final_sum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
