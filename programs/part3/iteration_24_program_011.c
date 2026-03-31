/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

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
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
    }
    
    /* ============================================
     * MAIN DEPENDENCY PATTERN LOOP
     * Creates multiple dependency types within nested loops
     * ============================================ */
    
    /* Outer loop with loop-carried dependency */
    for (i = 1; i < N; i++) {
        /* FLOW (RAW) Dependency: a[i] depends on a[i-1] from previous iteration */
        a[i] = a[i-1] + b[i];  /* Loop-carried flow dependency (distance=1) */
        
        /* Control flow to create basic block boundaries */
        if (i % 2 == 0) {
            /* ANTI (WAR) Dependency in true branch */
            x = a[i];          /* Read a[i] */
            a[i] = y + c[i];   /* Write a[i] - anti dependency with previous read */
            
            /* OUTPUT (WAW) Dependency */
            z = x * 2;         /* Register operation */
            z = y + 3;         /* Output dependency on z */
            
            /* Memory and register mix */
            b[i] = z + a[i];   /* Flow dependency on z and a[i] */
        } else {
            /* Different pattern in false branch */
            /* Another OUTPUT (WAW) Dependency */
            a[i] = x + i;      /* Write a[i] - output dependency with write in true branch */
            
            /* FLOW Dependency chain */
            y = a[i] * 2;
            z = y + 5;
            x = z - 3;
            
            /* ANTI Dependency */
            int temp = b[i];   /* Read b[i] */
            b[i] = temp + x;   /* Write b[i] - anti dependency */
        }
        
        /* ============================================
         * INNER LOOP with complex dependencies
         * Creates more edges in the DDG
         * ============================================ */
        for (j = 0; j < M; j++) {
            /* Inter-iteration dependencies in inner loop */
            if (j > 0) {
                /* Flow dependency across inner loop iterations */
                c[j] = c[j-1] + a[i];  /* Distance=1 in inner loop */
            }
            
            /* Multiple writes to same memory location */
            a[i] = a[i] + j;           /* Output dependency on a[i] from outer loop */
            a[i] = a[i] * 2;           /* Another output dependency */
            
            /* Register pressure and dependencies */
            int r1 = b[i] + j;
            int r2 = r1 * c[j];        /* Flow on r1 */
            int r3 = r2 - a[i];
            b[i] = r3 + r1;            /* Anti on b[i] from earlier read */
            
            /* Conditional to split basic blocks further */
            if (j % 3 == 0) {
                x = r2 + r3;
                y = x * 2;
            } else if (j % 3 == 1) {
                z = r3 - r2;
                x = z + 1;             /* Output on x from different branch */
            } else {
                y = r1 + r2;
                z = y - 3;             /* Output on z */
            }
        }
        
        /* Cross-iteration register dependencies */
        x = y + z;                     /* Depends on y,z from inner loop */
        y = x * 2;                     /* Flow on x */
    }
    
    /* ============================================
     * FINAL REDUCTION to prevent optimization
     * ============================================ */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional computation to create more scheduling opportunities */
    int prod = 1;
    for (i = 1; i < 32; i++) {
        /* Create dependencies with varying distances */
        prod = (prod * a[i]) % 1000;
        
        /* Conditional with dependencies */
        if (prod > 500) {
            b[i] = prod + b[i-1];      /* Flow on prod and b[i-1] */
        } else {
            b[i] = prod - b[i-1];      /* Different dependency pattern */
        }
    }
    
    printf("Product: %d\n", prod);
    
    return sum > 0 ? 0 : 1;  /* Use result in return value */
}
