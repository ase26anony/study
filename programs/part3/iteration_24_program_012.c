/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
    /* Declare arrays and scalars to create various dependency types */
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
     * Complex loop nest with multiple dependency types
     * ============================================ */
    
    /* Outer loop with loop-carried dependencies */
    for (i = 1; i < N; i++) {
        /* FLOW (RAW) dependency: a[i] depends on a[i-1] */
        a[i] = a[i-1] + b[i];  /* Loop-carried flow dep, distance=1 */
        
        /* ANTI (WAR) dependency: c[i] read before write */
        x = c[i];              /* Read c[i] */
        c[i] = a[i] * 2;       /* Write c[i] - anti dependency with above */
        
        /* OUTPUT (WAW) dependency on a[i] */
        if (i % 3 == 0) {
            /* Control flow creates basic block boundary */
            a[i] = x + y;      /* Another write to a[i] - output dep with line 37 */
            y = a[i] / 2;      /* Flow dep on a[i] */
        } else {
            /* Different path with its own dependencies */
            z = c[i] + 1;      /* Flow dep on c[i] from line 40 */
            a[i] = z * 3;      /* Output dep on a[i], flow dep on z */
        }
        
        /* More dependencies across iterations */
        b[i] = b[i-1] + i;     /* Loop-carried flow dep on b, distance=1 */
    }
    
    /* ============================================
     * Nested loop with register and memory mix
     * ============================================ */
    int sum = 0;
    for (i = 0; i < M; i++) {
        int temp = 0;
        for (j = 0; j < M; j++) {
            /* Complex addressing with multiple dependencies */
            int idx = (i * M + j) % N;
            
            /* Flow dependency chain through registers */
            temp = a[idx] + b[idx];
            temp = temp * c[idx];      /* Flow dep on temp */
            sum += temp;               /* Flow dep on temp, anti on sum */
            
            /* Anti dependency with array */
            int old_val = a[idx];      /* Read a[idx] */
            a[idx] = temp + old_val;   /* Write a[idx] - anti dep, flow on temp & old_val */
            
            /* Output dependency in inner loop */
            if (j % 2 == 0) {
                temp = b[idx] - c[idx]; /* Output dep on temp */
            } else {
                temp = c[idx] - b[idx]; /* Output dep on temp */
            }
        }
        
        /* Loop-carried dependency through sum */
        y = sum + i;                   /* Flow dep on sum, distance=1 in outer loop */
        sum = y % 100;                 /* Output dep on sum, flow on y */
    }
    
    /* ============================================
     * Additional dependency patterns
     * ============================================ */
    int r1 = 0, r2 = 0, r3 = 0;
    
    /* Register-only dependency chain */
    for (i = 0; i < 100; i++) {
        r1 = r2 + r3;      /* Flow deps on r2, r3 */
        r2 = r1 * 2;       /* Flow dep on r1, output dep on r2 */
        r3 = r2 - i;       /* Flow dep on r2, output dep on r3 */
    }
    
    /* Mixed memory/register with control flow */
    for (i = 0; i < N; i++) {
        if (a[i] > 50) {
            x = b[i] + c[i];
            a[i] = x * 2;      /* Flow dep on x, output dep on a[i] */
            b[i] = a[i] - 1;   /* Flow dep on a[i], output dep on b[i] */
        } else {
            y = c[i] - b[i];
            a[i] = y / 2;      /* Flow dep on y, output dep on a[i] */
            c[i] = a[i] + 3;   /* Flow dep on a[i], output dep on c[i] */
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int total = 0;
    for (i = 0; i < N; i++) {
        total += a[i] + b[i] + c[i];
    }
    
    total += sum + x + y + z + r1 + r2 + r3;
    
    printf("Result: %d\n", total);
    return total % 100;  /* Ensure computation isn't optimized away */
}
