/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main(void) {
    /* Declare arrays and scalars to create various dependencies */
    int a[N], b[N], c[N], d[N];
    int x = 0, y = 0, z = 0, w = 0;
    int i, j, k;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
        d[i] = 0;
    }
    
    /* ============================================
     * Complex loop nest with multiple dependencies
     * ============================================ */
    
    /* Outer loop - creates loop-carried dependencies */
    for (i = 1; i < N; i++) {
        /* FLOW (RAW) dependency with distance 1 - loop-carried */
        a[i] = a[i-1] + b[i];  /* Reads a[i-1], writes a[i] */
        
        /* ANTI (WAR) dependency within same iteration */
        x = c[i];              /* Reads c[i] */
        c[i] = y + i;          /* Writes c[i] - anti-dependency with previous */
        
        /* OUTPUT (WAW) dependency */
        d[i] = x * 2;          /* First write to d[i] */
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* Another FLOW dependency inside conditional */
            z = a[i] + 1;      /* Reads a[i] written above */
            d[i] = z + 2;      /* Second write to d[i] - output dependency */
            
            /* Memory and register mix */
            w = b[i] * 3;      /* Register operation */
            b[i] = w / 2;      /* Memory write - anti with previous read */
        } else if (i % 3 == 1) {
            /* Different path with dependencies */
            y = c[i] + d[i];   /* Flow from c[i] and d[i] */
            a[i] = y - 1;      /* Output dependency with earlier a[i] write */
        } else {
            /* Third path */
            d[i] = b[i] + c[i]; /* Another output to d[i] */
            x = d[i] * 2;       /* Flow from d[i] */
        }
        
        /* Nested inner loop for more complexity */
        for (j = 0; j < M; j++) {
            /* Register pressure and dependencies */
            int temp = (i * j) % 7;
            
            /* Flow dependency chain */
            x = x + temp;
            y = y + x;
            z = z + y;
            
            /* Anti dependency */
            w = z;
            z = temp * 2;
            
            /* Small conditional inside inner loop */
            if (j % 5 == 0) {
                x = w + 1;      /* Flow from w */
            }
        }
        
        /* Cross-iteration dependency with different distance */
        if (i > 10) {
            /* Flow dependency with distance 10 */
            b[i] = b[i-10] + 1;
        }
    }
    
    /* ============================================
     * Second loop with different patterns
     * ============================================ */
    for (i = 0; i < N; i += 2) {
        /* Paired operations creating web of dependencies */
        int t1 = a[i] + b[i];     /* Flow from a[i], b[i] */
        int t2 = c[i] + d[i];     /* Flow from c[i], d[i] */
        
        /* Output dependency */
        a[i] = t1 * t2;           /* Overwrites a[i] */
        
        /* Anti dependency chain */
        t1 = t2 + 1;              /* Uses t2 */
        t2 = t1 * 2;              /* Anti: overwrites t2 after reading t1 */
        
        /* Write to memory with flow to next iteration */
        if (i + 1 < N) {
            c[i+1] = t2 + a[i];   /* Flow from t2 and a[i], loop-carried */
        }
    }
    
    /* ============================================
     * Reduction to prevent dead code elimination
     * ============================================ */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i] + d[i];
        sum += x + y + z + w;  /* Include scalars */
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", sum);
    
    /* Additional volatile operations to force memory dependencies */
    volatile int *volatile_ptr = &sum;
    *volatile_ptr = *volatile_ptr + 1;
    
    return sum > 0 ? 0 : 1;
}
