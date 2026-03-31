/* Program to trigger DDG edge creation in GCC's ddg.cc */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
    /* Declare arrays and scalars to create various dependencies */
    int a[N], b[N], c[N], d[N];
    int x = 0, y = 0, z = 0, w = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
        d[i] = 0;
    }
    
    /* 
     * Complex loop nest with multiple dependency types
     * This creates a web of dependencies for DDG construction
     */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep: a[i-1] -> a[i] */
        
        /* Anti dependency (WAR) within same iteration */
        x = c[i];              /* Read c[i] */
        c[i] = y + i;          /* Write c[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        d[i] = x * 2;          /* First write to d[i] */
        if (i % 3 == 0) {
            d[i] = z + 1;      /* Second write to d[i] - output dep */
            z = d[i-1];        /* Flow dep across iterations */
        }
        
        /* Nested loop to increase complexity */
        for (j = 0; j < M; j++) {
            /* Register and memory mix */
            w = w + a[i] + b[j % N];
            
            /* Conditional creates basic block boundaries */
            if (j % 2 == 0) {
                /* More dependencies inside conditional */
                y = y + c[i] * 2;
                b[j % N] = y + w;  /* Anti dep on y from previous stmt */
            } else {
                /* Alternative path with different deps */
                x = x + d[i];
                a[i] = a[i] + x;   /* Flow dep on x, output dep on a[i] */
            }
            
            /* Cross-iteration dependency in inner loop */
            if (j > 0) {
                c[i] = c[i] + b[(j-1) % N];  /* Flow dep across inner iterations */
            }
        }
        
        /* Control flow merges here */
        /* Create register pressure and more deps */
        int temp = a[i] + b[i] + c[i] + d[i];
        if (temp % 5 == 0) {
            z = z + temp;
        } else {
            w = w - temp;
        }
        
        /* Another anti dependency pattern */
        int old_x = x;
        x = temp / 2;
        y = old_x + x;  /* Flow dep on old_x, anti dep on x */
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
    }
    sum += x + y + z + w;
    
    printf("Result: %d\n", sum);
    return sum;
}
