/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex dependency patterns to create various DDG edges */
void process_data(int *a, int *b, int *c, int n) {
    int i, j;
    int temp_reg = 0;
    int accum = 0;
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];
        
        /* Control flow to create basic block boundaries */
        if (i % 3 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            accum += temp_reg;
            temp_reg = c[i] * 2;
            
            /* OUTPUT (WAW) dependency on same memory location */
            b[i] = accum;
            b[i] = temp_reg + 1;  // WAW on b[i]
        } else if (i % 3 == 1) {
            /* Another ANTI dependency pattern */
            int tmp = a[i];
            a[i] = b[i] + tmp;  // WAR on a[i]
            
            /* Flow dependency through register */
            temp_reg = tmp * 3;
            accum = temp_reg + accum;  // RAW on temp_reg
        } else {
            /* Complex memory and register mix */
            c[i] = a[i] + b[i];
            
            /* Multiple flow dependencies */
            temp_reg = c[i] - a[i];
            accum = temp_reg + b[i];
            
            /* Output dependency in else block */
            b[i] = accum;
        }
        
        /* Cross-iteration register dependency (loop-carried) */
        accum = accum + temp_reg;
    }
    
    /* Nested loop with different distance */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* 2D array-like access with flow dependencies */
            int idx = i * M + j;
            
            /* Flow dependency with distance > 0 in inner loop */
            c[idx] = c[idx - 1] + a[i];
            
            /* Anti dependency in inner loop */
            int old_val = b[idx];
            b[idx] = c[idx] * 2;  // WAR on b[idx]
            a[i] += old_val;
        }
    }
}

/* Another function with different patterns */
void vector_operations(int *x, int *y, int *z, int n) {
    int i;
    
    /* Unrolled loop with dependencies */
    for (i = 0; i < n - 4; i += 4) {
        /* Multiple flow dependencies in unrolled loop */
        z[i] = x[i] + y[i];
        z[i+1] = z[i] + x[i+1];  // Flow from z[i]
        z[i+2] = z[i+1] + y[i+2]; // Flow from z[i+1]
        z[i+3] = z[i+2] + x[i+3]; // Flow from z[i+2]
        
        /* Anti dependencies between unrolled iterations */
        int t1 = x[i];
        x[i] = y[i] * 2;  // WAR on x[i]
        y[i] = t1 + z[i]; // Flow from t1 and anti on y[i]
    }
}

int main() {
    /* Allocate and initialize data */
    int *array_a = (int*)malloc(N * M * sizeof(int));
    int *array_b = (int*)malloc(N * M * sizeof(int));
    int *array_c = (int*)malloc(N * M * sizeof(int));
    
    if (!array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N * M; i++) {
        array_a[i] = i % 100;
        array_b[i] = (i + 1) % 100;
        array_c[i] = (i * 2) % 100;
    }
    
    /* Process data with complex dependencies */
    process_data(array_a, array_b, array_c, N);
    
    /* More operations with different dependency patterns */
    vector_operations(array_a, array_b, array_c, N * M / 4);
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < N * M; i++) {
        sum += array_a[i] + array_b[i] + array_c[i];
    }
    
    printf("Result: %d\n", sum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
