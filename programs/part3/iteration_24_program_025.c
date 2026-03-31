/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main(void) {
    /* Declare arrays and scalars to create various dependencies */
    int a[N], b[N], c[N];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        b[i] = i;
        c[i] = N - i;
    }
    
    /* Complex loop nest with multiple dependency types */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow (RAW) dependency - distance = 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep on a[i-1] from previous iteration */
        
        /* Anti (WAR) dependency within same iteration */
        x = a[i];              /* Read a[i] */
        a[i] = c[i] * 2;       /* Write a[i] - anti dependency with previous */
        
        /* Output (WAW) dependency */
        y = b[i] + x;          /* Use x */
        if (y > M) {           /* Control flow creates basic block boundary */
            /* Memory and register dependencies in conditional block */
            z = a[i] * 3;      /* Flow dep on a[i] */
            b[i] = z + i;      /* Output dep on b[i], flow dep on z */
        } else {
            /* Alternative path with different dependencies */
            z = c[i] / 2;      /* Flow dep on c[i] */
            b[i] = z - i;      /* Output dep on b[i], flow dep on z */
        }
        
        /* Another anti dependency */
        int temp = b[i];       /* Read b[i] */
        b[i] = temp + y;       /* Write b[i] - anti dependency */
        
        /* Nested loop to increase complexity */
        for (j = 0; j < 4; j++) {
            /* Create register dependencies */
            x = x + j;
            y = y - j;
            /* Memory dependency with outer loop */
            c[i] = c[i] + x * y;
        }
        
        /* Cross-iteration anti dependency */
        if (i > 2) {
            /* Read value written in previous iteration */
            int prev_val = a[i-2];  /* Distance = 2 flow dep */
            a[i] = a[i] + prev_val; /* Flow dep on a[i], anti on prev read */
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to ensure computation isn't optimized away */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
