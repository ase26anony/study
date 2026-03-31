/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars to create various dependency types */
    int a[SIZE], b[SIZE], c[SIZE];
    int x, y, z, sum = 0;
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
        a[i] = a[i-1] + b[i];  /* Flow dep: reads a[i-1], writes a[i] */
        
        /* Anti dependency (WAR) within same iteration */
        x = a[i];              /* Read a[i] */
        a[i] = c[i] * 3;       /* Write a[i] - anti dep with previous read */
        
        /* Output dependency (WAW) */
        y = b[i] + x;          /* Intermediate computation */
        b[i] = y * 2;          /* First write to b[i] */
        
        /* Conditional to create basic block boundaries */
        if (i % 3 == 0) {
            /* More dependencies inside conditional block */
            z = a[i] + b[i];   /* Flow dep on a[i] and b[i] */
            c[i] = z / 2;      /* Output dep on c[i] */
            
            /* Anti dependency across condition */
            int temp = c[i-1]; /* Read c[i-1] */
            c[i-1] = temp + 1; /* Write c[i-1] - anti dep */
        } else {
            /* Alternative path with different dependencies */
            z = b[i] - a[i];   /* Flow dep on a[i] and b[i] */
            c[i] = z * 3;      /* Output dep on c[i] */
            
            /* Register pressure to force spills */
            int r1 = a[i] + 1;
            int r2 = r1 * 2;
            int r3 = r2 - b[i];
            int r4 = r3 + c[i];
            a[i] = r4;         /* Output dep on a[i] */
        }
        
        /* Cross-iteration anti dependency with distance 2 */
        if (i > 2) {
            b[i] = c[i-2] + a[i]; /* Reads c[i-2] from 2 iterations ago */
        }
        
        /* Memory and register mix for varied data types */
        sum += a[i] + b[i] + c[i]; /* Reduction to prevent elimination */
    }
    
    /* Nested loop for additional complexity */
    for (i = 0; i < SIZE/2; i++) {
        for (j = 0; j < SIZE/2; j++) {
            /* Inter-iteration dependencies in nested loop */
            if (j > 0) {
                a[i*2 + j] = a[i*2 + j-1] + b[j]; /* Flow dep in j-loop */
            }
            
            /* Multiple writes to same memory location */
            c[j] = i + j;
            c[j] = c[j] * 2;  /* Output dependency */
            
            /* Register chain */
            int reg1 = a[i*2 + j];
            int reg2 = reg1 * reg1;
            int reg3 = reg2 + b[j];
            b[j] = reg3;      /* Anti dep if b[j] was read earlier */
        }
        sum += c[i];
    }
    
    /* Final computation using results */
    int final = 0;
    for (i = 0; i < SIZE; i++) {
        final += a[i] % 7 + b[i] % 5 + c[i] % 3;
    }
    
    /* Use result to prevent dead code elimination */
    return final + sum;
}
