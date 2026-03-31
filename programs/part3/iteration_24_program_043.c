/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars to create various dependency types */
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    /* Complex loop nest with multiple dependency patterns */
    for (i = 1; i < SIZE - 1; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  /* Flow dep: a[i-1] -> a[i] */
        
        /* Anti dependency (WAR) */
        x = a[i];              /* Read a[i] */
        a[i] = y + c[i];       /* Write a[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        y = x * 2;             /* Write y */
        if (i % 2 == 0) {      /* Control flow creates basic block boundary */
            /* Different basic block with more dependencies */
            y = z + 1;         /* Another write to y - output dep */
            z = b[i] - 1;      /* Flow dep through b[i] */
        } else {
            /* Alternative path with register dependencies */
            int temp = y;      /* Register anti dep (WAR) */
            y = temp + c[i];   /* Register flow dep */
            temp = z;          /* More register ops */
            z = temp * 3;
        }
        
        /* Memory anti dependency across arrays */
        b[i] = a[i] + x;       /* Read a[i], write b[i] */
        
        /* Nested loop for additional complexity */
        for (j = 0; j < 4; j++) {
            /* Register pressure and dependencies */
            int r1 = y + j;
            int r2 = r1 * 2;   /* Flow dep in registers */
            y = r2 - 1;        /* Output dep on y */
            c[j] = r2;         /* Memory write with flow dep */
        }
        
        /* Output dependency on memory */
        a[i] = x + y;          /* Another write to a[i] - output dep */
        
        /* Complex expression with multiple dependencies */
        c[i] = (a[i] * b[i]) + (c[i-1] - x); /* Mix of flow and anti deps */
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to ensure computation isn't optimized away */
    return sum % 256;
}
