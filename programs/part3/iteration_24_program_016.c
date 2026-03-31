/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Complex dependency patterns to create various DDG edges */
void compute_with_dependencies(int *a, int *b, int *c, int *d, int n) {
    int i, j;
    int reg1, reg2, reg3;
    
    /* Initialize */
    for (i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
        c[i] = i * 2;
        d[i] = 0;
    }
    
    /* Main computation with mixed dependencies */
    for (i = 1; i < n - 1; i++) {
        /* FLOW (RAW) dependency - memory to memory */
        a[i] = b[i] + c[i];          /* Statement 1 */
        
        /* FLOW (RAW) dependency - memory to register */
        reg1 = a[i] * 2;             /* Statement 2 - depends on Statement 1 */
        
        /* ANTI (WAR) dependency */
        reg2 = b[i];                 /* Statement 3 */
        b[i] = reg1 + reg2;          /* Statement 4 - anti-dep with Statement 3 */
        
        /* OUTPUT (WAW) dependency */
        c[i] = reg1 + i;             /* Statement 5 */
        c[i] = reg2 * 3;             /* Statement 6 - output-dep with Statement 5 */
        
        /* Control flow to create basic block boundaries */
        if (i % 3 == 0) {
            /* More FLOW dependencies in conditional block */
            d[i] = a[i] + b[i];      /* Statement 7 */
            reg3 = d[i] * c[i];      /* Statement 8 */
        } else if (i % 3 == 1) {
            /* Different dependency pattern */
            d[i] = b[i] - a[i];      /* Statement 9 */
            reg3 = c[i] / 2;         /* Statement 10 */
        } else {
            /* Another pattern */
            d[i] = a[i] * b[i];      /* Statement 11 */
            reg3 = 0;                /* Statement 12 */
        }
        
        /* Loop-carried FLOW dependency (distance = 1) */
        a[i+1] = a[i] + reg3;        /* Statement 13 - depends on previous iteration */
        
        /* Loop-carried ANTI dependency */
        reg2 = c[i-1];               /* Statement 14 - anti-dep with c[i-1] from prev iter */
        c[i] = reg2 + 1;             /* Statement 15 */
    }
    
    /* Nested loop with different dependency pattern */
    for (i = 0; i < n/2; i++) {
        for (j = 0; j < M; j++) {
            /* Complex index calculations create address dependencies */
            int idx = (i * 3 + j) % n;
            
            /* Cross-iteration FLOW dependency in inner loop */
            b[idx] = a[idx] + j;     /* Statement 16 */
            
            /* OUTPUT dependency in inner loop */
            a[idx] = i + j;          /* Statement 17 */
            a[idx] = b[idx] * 2;     /* Statement 18 - output-dep with Statement 17 */
            
            /* Register pressure to force spill/reload */
            int temp1 = a[idx] + b[idx];
            int temp2 = temp1 * c[idx];
            int temp3 = temp2 - d[idx];
            d[idx] = temp3 + i;
        }
    }
}

/* Another function with recurrence for modulo scheduling */
void recurrence_pattern(int *x, int *y, int n) {
    int i;
    
    /* Strongly connected component: recurrence with loop-carried dep */
    x[0] = y[0];
    for (i = 1; i < n; i++) {
        /* Multiple loop-carried dependencies */
        x[i] = x[i-1] + y[i];        /* FLOW, distance=1 */
        y[i] = y[i-1] * x[i];        /* FLOW, distance=1, and anti-dep with x[i] */
    }
    
    /* Reduction with dependency chain */
    int sum = 0;
    for (i = 0; i < n; i++) {
        sum += x[i] * y[i];          /* FLOW dependency on sum */
    }
    
    /* Use result to prevent optimization */
    x[0] = sum;
}

int main() {
    int *a = malloc(N * sizeof(int));
    int *b = malloc(N * sizeof(int));
    int *c = malloc(N * sizeof(int));
    int *d = malloc(N * sizeof(int));
    int *x = malloc(N * sizeof(int));
    int *y = malloc(N * sizeof(int));
    
    if (!a || !b || !c || !d || !x || !y) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Multiple computation patterns to increase DDG complexity */
    compute_with_dependencies(a, b, c, d, N);
    recurrence_pattern(x, y, N/2);
    
    /* Cross-dependencies between arrays */
    for (int i = 0; i < N; i++) {
        a[i] = a[i] + x[i % (N/2)];
        b[i] = b[i] * y[i % (N/2)];
    }
    
    /* Final reduction to ensure computation isn't optimized away */
    int result = 0;
    for (int i = 0; i < N; i++) {
        result += a[i] + b[i] + c[i] + d[i] + x[i % (N/2)] + y[i % (N/2)];
    }
    
    printf("Result: %d\n", result);
    
    free(a);
    free(b);
    free(c);
    free(d);
    free(x);
    free(y);
    
    return 0;
}
