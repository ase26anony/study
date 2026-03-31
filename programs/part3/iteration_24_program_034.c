/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Arrays to create memory dependencies */
    int a[SIZE], b[SIZE], c[SIZE];
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = SIZE - i;
    }
    
    /* Complex loop nest with various dependency types */
    for (i = 1; i < SIZE - 1; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep: reads a[i-1], writes a[i] */
        
        /* Anti dependency (WAR) within same iteration */
        int temp = c[i];        /* Read c[i] */
        c[i] = a[i] * 2;        /* Write c[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        int x = temp + i;       /* Compute something */
        x = x * 3;              /* Output dep: overwrites x */
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* More dependencies inside conditional block */
            b[i] = c[i-1] + x;  /* Flow dep with c[i-1] from previous iteration */
            
            /* Anti dependency across iterations */
            int y = a[i];       /* Read a[i] */
            a[i] = y + b[i];    /* Write a[i] - anti dep with read above */
        } else if (i % 3 == 1) {
            /* Different path with its own dependencies */
            c[i] = b[i] + a[i-2]; /* Flow dep with distance 2 */
            
            /* Register pressure to force scheduling decisions */
            int r1 = a[i];
            int r2 = b[i];
            int r3 = c[i];
            int r4 = r1 + r2;
            int r5 = r3 * r4;
            a[i] = r5;
        } else {
            /* Third path with output dependencies */
            int z = i * 2;
            z = z + 5;          /* Output dep on z */
            z = z * 3;          /* Another output dep */
            b[i] = z;
        }
        
        /* Cross-iteration anti dependency */
        int anti_temp = b[i+1]; /* Read b[i+1] (future iteration's write) */
        b[i] = anti_temp + 1;   /* Write b[i] */
    }
    
    /* Nested loop for additional complexity */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < 4; j++) {
            /* Recurrence with small distance in inner loop */
            if (j > 0) {
                a[i] = a[i] + b[i] * j;  /* Self-dependency */
            }
            
            /* Memory and register mix */
            int reg1 = a[i];
            int reg2 = b[i];
            int reg3 = reg1 * reg2;
            c[i] = c[i] + reg3;  /* Flow dep on c[i] */
        }
        sum += a[i] + b[i] + c[i];
    }
    
    /* Prevent dead code elimination */
    volatile int result = sum;
    
    return result % 256;  /* Ensure computation isn't optimized away */
}
