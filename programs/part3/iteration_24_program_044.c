/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
    int i, j, k;
    int a[N], b[N], c[N], d[N];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
        d[i] = 0;
    }
    
    /* Complex loop nest with various dependencies */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep: a[i-1] -> a[i] */
        
        /* Anti dependency (WAR) within same iteration */
        x = c[i];              /* Read c[i] */
        c[i] = y + z;          /* Write c[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        d[i] = x * 2;          /* Write d[i] */
        if (i % 2 == 0) {
            /* Control flow creates basic block boundary */
            d[i] = y + 1;      /* Another write to d[i] - output dep */
            y = d[i-1];        /* Flow dep across iterations */
        } else {
            /* Different path with its own dependencies */
            z = a[i] + b[i-1]; /* Flow deps: a[i], b[i-1] -> z */
        }
        
        /* Memory and register mix */
        for (j = 0; j < M; j++) {
            /* Nested loop with reduction */
            sum += a[i] * j;   /* Flow dep: a[i] -> sum */
            
            /* Anti dependency in inner loop */
            int temp = b[j % N];
            b[j % N] = temp + 1; /* Anti dep: read then write b[...] */
            
            /* Output dependency in inner loop */
            c[j % N] = temp;
            c[j % N] = temp * 2; /* Output dep: two writes to c[...] */
        }
        
        /* Cross-iteration anti dependency */
        y = a[i];              /* Write y */
        a[i] = x + z;          /* Read x, z; anti dep on y not used here */
    }
    
    /* Additional loop with pointer aliasing potential */
    int *p = a;
    int *q = b;
    for (k = 0; k < N/2; k++) {
        /* Potential memory dependencies through pointers */
        *p = *q + *(p+1);     /* Flow deps: *q, *(p+1) -> *p */
        p++;
        q++;
        
        /* Register dependencies */
        int r1 = *p;
        int r2 = r1 * 2;      /* Flow dep: r1 -> r2 */
        int r3 = r2 + r1;     /* Flow deps: r1, r2 -> r3 */
        *q = r3;              /* Flow dep: r3 -> *q */
    }
    
    /* Final reduction to prevent dead code elimination */
    int total = 0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i] + c[i] + d[i];
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d (sum: %d)\n", total, sum);
    
    return total > 0 ? 0 : 1;
}
