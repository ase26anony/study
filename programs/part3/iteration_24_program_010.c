/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
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
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep: a[i-1] -> a[i] */
        
        /* Anti dependency (WAR) within same iteration */
        x = a[i];              /* Read a[i] */
        a[i] = y + c[i];       /* Write a[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        y = x * 2;             /* Write y */
        if (i % 3 == 0) {
            /* Control flow creates basic block boundary */
            y = z + 1;         /* Another write to y - output dep */
            z = b[i] - 1;
        } else {
            /* Different path with its own dependencies */
            z = y + c[i];      /* Flow dep: y -> z */
        }
        
        /* Nested loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Memory and register mix */
            int temp = b[j % N];
            
            /* Multiple dependencies in nested loop */
            c[j % N] = temp + i;  /* Flow: temp -> c[] */
            temp = c[j % N];      /* Anti: c[] -> temp */
            
            /* Output dependency in nested loop */
            b[j % N] = i * j;
            if (j % 2 == 0) {
                b[j % N] = temp + 1;  /* Another write to b[] - output dep */
            }
        }
        
        /* More flow dependencies across iterations */
        if (i > 2) {
            /* Flow dep with distance 2 */
            int t = a[i-2] + a[i-1];  /* Uses values from 2 previous iterations */
            a[i] += t;
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i % N] + c[i % N];
    }
    
    /* Use result to ensure computation isn't optimized away */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
