/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex function with various dependency patterns */
int compute(int *a, int *b, int *c, int n) {
    int i, j;
    int sum = 0;
    int temp_reg;
    int scalar = 42;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency: read after write */
        temp_reg = a[i-1] + scalar;      /* Read a[i-1] */
        a[i] = temp_reg * 2;             /* Write a[i] - creates flow dep with next iteration */
        
        /* ANTI (WAR) dependency: write after read */
        int read_val = b[i];              /* Read b[i] */
        b[i] = scalar + i;                /* Write b[i] - anti dep with previous read */
        
        /* OUTPUT (WAW) dependency: write after write */
        c[i] = read_val * 3;              /* Write c[i] */
        if (i % 2 == 0) {
            /* Control flow creates basic block boundary */
            c[i] = temp_reg + 1;          /* Another write to c[i] - output dep */
        }
        
        /* Loop-carried dependency with distance > 0 */
        if (i > 3) {
            /* Cross-iteration flow dependency */
            a[i] = a[i-3] + b[i-2];       /* Distance 3 flow dep */
        }
        
        /* Register pressure to force spills */
        int r1 = a[i] * 2;
        int r2 = b[i] + r1;
        int r3 = c[i] - r2;
        int r4 = r1 * r3;
        int r5 = r2 / (r4 + 1);
        sum += r5;
    }
    
    /* Nested loop with different access patterns */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* Strided access pattern */
            int idx = i * M + j;
            
            /* Complex dependency web */
            int val1 = a[idx % n];
            int val2 = b[(idx + 1) % n];
            
            /* Multiple consumers of same value */
            c[idx % n] = val1 + val2;
            a[(idx + 2) % n] = val1 * val2;
            b[(idx + 3) % n] = c[idx % n] - a[(idx + 2) % n];
            
            /* Conditional creates control flow */
            if (val1 > val2) {
                /* More dependencies in this path */
                temp_reg = c[idx % n] + 1;
                a[idx % n] = temp_reg;
            } else {
                /* Alternative path with different deps */
                temp_reg = b[idx % n] - 1;
                c[idx % n] = temp_reg;
            }
            
            sum += temp_reg;
        }
    }
    
    return sum;
}

/* Function with pointer aliasing potential */
void process_arrays(int * restrict x, int * restrict y, int *z, int n) {
    int i;
    
    /* Restrict helps but we still mix pointers */
    for (i = 1; i < n; i++) {
        /* Flow dependencies through memory */
        x[i] = y[i-1] + z[i];
        y[i] = x[i] * 2;
        z[i] = y[i] - x[i-1];
        
        /* Anti dependency */
        int tmp = x[i];
        x[i] = z[i] + 1;  /* WAR on x[i] */
        
        /* Output dependency */
        y[i] = tmp * 3;
        if (i % 3 == 0) {
            y[i] = tmp + 5;  /* WAW on y[i] */
        }
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
        array3[i] = (i * 3) % 7;
    }
    
    /* Call functions that create complex dependencies */
    int result1 = compute(array1, array2, array3, N);
    
    /* Process with restrict qualifiers */
    process_arrays(array1, array2, array3, N);
    
    /* Final reduction to prevent dead code elimination */
    int final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i] + array3[i];
    }
    
    final_sum += result1;
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", final_sum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
