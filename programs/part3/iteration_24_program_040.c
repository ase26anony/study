/* ddg_test.c - Program to trigger DDG edge creation in GCC */
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
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* ANTI (WAR) dependency: temp_reg read before write */
            c[i] = temp_reg + i;
            temp_reg = b[i] * 2;  /* Write to temp_reg */
            
            /* OUTPUT (WAW) dependency on a[i] */
            a[i] = c[i] * 3;  /* Second write to a[i] */
        } else if (i % 3 == 1) {
            /* Different path with different dependencies */
            /* FLOW dependency through memory */
            int t = a[i] + 1;  /* Read a[i] */
            b[i] = t * 2;      /* Write b[i] */
            
            /* ANTI dependency through register */
            accum = accum + t; /* Read t */
            t = i * 7;         /* Write t */
        } else {
            /* Third path with output dependencies */
            /* OUTPUT (WAW) on c[i] */
            c[i] = i * 5;
            c[i] = i * 11;     /* Second write to c[i] */
            
            /* FLOW dependency chain */
            temp_reg = temp_reg + a[i];
            accum = accum + temp_reg;
        }
        
        /* Loop-carried anti dependency across iterations */
        /* WAR: b[i] written above, read here in next iteration */
    }
    
    /* Nested loop with different dependency patterns */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* 2D loop-carried flow dependency */
            a[i*M + j] = a[i*M + j-1] + c[j];
            
            /* Register pressure to force spill/reload */
            int r1 = a[i*M + j] * 2;
            int r2 = r1 + b[j];
            int r3 = r2 * 3;
            int r4 = r3 - c[j];
            int r5 = r4 / 2;
            
            /* Anti dependency chain */
            b[j] = r5 + 1;     /* Write b[j] */
            c[j] = b[j] * 2;   /* Read b[j], write c[j] */
            a[i*M + j] = c[j]; /* Read c[j], write a[i*M+j] */
        }
    }
    
    /* Prevent dead code elimination */
    printf("Accumulator: %d\n", accum);
}

/* Main function with initialization */
int main() {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(N * sizeof(int));
    int *array_b = (int*)malloc(N * sizeof(int));
    int *array_c = (int*)malloc(N * sizeof(int));
    
    if (!array_a || !array_b || !array_c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        array_a[i] = i;
        array_b[i] = i * 2;
        array_c[i] = i * 3;
    }
    
    /* Process data with complex dependencies */
    process_data(array_a, array_b, array_c, N);
    
    /* Final reduction to ensure computation isn't optimized away */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += array_a[i] + array_b[i] + array_c[i];
    }
    
    printf("Final sum: %d\n", sum);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    
    return 0;
}
