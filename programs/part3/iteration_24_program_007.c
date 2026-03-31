/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex dependency patterns to create various DDG edges */
void create_ddg_edges(int *restrict a, int *restrict b, int *restrict c, 
                      int *restrict d, int n) {
    int i, j;
    int temp_reg = 0;
    int scalar1 = 1, scalar2 = 2, scalar3 = 3;
    
    /* Loop with multiple dependency types */
    for (i = 1; i < n; i++) {
        /* FLOW (RAW) dependency - memory to memory */
        a[i] = b[i] + c[i];           /* Source node */
        d[i] = a[i] * 2;              /* Dest node with flow dep from a[i] */
        
        /* ANTI (WAR) dependency */
        scalar1 = a[i-1];             /* Read a[i-1] */
        a[i-1] = scalar2 + i;         /* Write a[i-1] - anti dep with previous */
        
        /* OUTPUT (WAW) dependency */
        temp_reg = b[i] * 3;          /* Intermediate computation */
        b[i] = temp_reg + scalar3;    /* Write b[i] - output dep if another write exists */
        
        /* Control flow to create basic block boundaries */
        if (i % 3 == 0) {
            /* Different dependency pattern in this block */
            scalar2 = d[i] + scalar1;  /* Flow dep from d[i] and scalar1 */
            c[i] = scalar2 * i;        /* Flow dep from scalar2 */
            
            /* Another anti dependency */
            temp_reg = c[i-1];         /* Read c[i-1] */
            c[i-1] = temp_reg + 1;     /* Write c[i-1] */
        } else if (i % 3 == 1) {
            /* Alternative path with output dependencies */
            b[i] = scalar1 * 2;        /* Another write to b[i] - output dep */
            scalar3 = b[i] + a[i];     /* Flow dep from both */
        } else {
            /* Third path with register dependencies */
            scalar1 = scalar2 + scalar3;
            scalar2 = scalar1 * i;
        }
        
        /* Loop-carried dependency (distance = 1) */
        c[i] = c[i-1] + b[i];         /* Flow dep with distance > 0 */
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < n/2; i++) {
        for (j = 1; j < M; j++) {
            /* Cross-iteration dependencies in nested loop */
            a[i*2 + j%2] = b[i] + a[i*2 + (j-1)%2];  /* Complex addressing */
            
            /* Multiple register dependencies */
            temp_reg = scalar1 + scalar2;
            scalar1 = scalar2 + temp_reg;
            scalar2 = temp_reg * j;
            
            /* Conditional with dependencies */
            if (j % 4 == 0) {
                d[i] = a[i*2] + scalar1;
            }
        }
    }
}

/* Function with recurrence for strong loop-carried dependencies */
int recurrence_pattern(int *arr, int n) {
    int sum = 0;
    int i;
    
    /* Multiple accumulators with dependencies */
    int acc1 = arr[0];
    int acc2 = arr[1];
    int acc3 = arr[2];
    
    for (i = 3; i < n; i++) {
        /* Chain of dependencies */
        acc1 = acc1 + arr[i];      /* Simple recurrence */
        acc2 = acc1 * 2 + acc2;    /* Depends on acc1 and previous acc2 */
        acc3 = acc2 - acc3 + i;    /* Depends on acc2 and previous acc3 */
        
        /* Cross-iteration memory dependencies */
        arr[i-2] = arr[i-1] + acc3;  /* Flow dep with distance 1 */
        arr[i-1] = acc1 + acc2;      /* Flow dep, also creates anti dep */
        
        /* Register pressure to force spills */
        int r1 = acc1 + i;
        int r2 = acc2 * i;
        int r3 = acc3 / (i+1);
        int r4 = r1 + r2;
        int r5 = r3 * r4;
        int r6 = r5 - arr[i-2];
        
        sum += r6;
    }
    
    return sum + acc1 + acc2 + acc3;
}

int main() {
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    int *c = (int*)malloc(N * sizeof(int));
    int *d = (int*)malloc(N * sizeof(int));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
        d[i] = i * 3;
    }
    
    /* Create complex DDG edges */
    create_ddg_edges(a, b, c, d, N);
    
    /* Create recurrence patterns */
    int result = recurrence_pattern(a, N);
    
    /* Use results to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
