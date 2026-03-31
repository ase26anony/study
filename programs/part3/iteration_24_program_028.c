/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars for various dependency types */
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    /* ============================================
     * MAIN LOOP WITH COMPLEX DEPENDENCY PATTERNS
     * ============================================ */
    
    /* Outer loop for loop-carried dependencies */
    for (i = 1; i < SIZE - 1; i++) {
        /* ============================================
         * DEPENDENCY GROUP 1: Loop-carried flow (RAW)
         * ============================================ */
        /* Flow dependency across iterations (distance = 1) */
        a[i] = a[i - 1] + b[i];  /* RAW: a[i-1] -> a[i] */
        
        /* ============================================
         * DEPENDENCY GROUP 2: Anti (WAR) dependencies
         * ============================================ */
        /* Anti dependency within same iteration */
        x = a[i];                /* Read a[i] */
        a[i] = y + c[i];         /* Write a[i] - WAR: x = a[i] -> a[i] = ... */
        
        /* ============================================
         * DEPENDENCY GROUP 3: Output (WAW) dependencies
         * ============================================ */
        /* Output dependency on same memory location */
        c[i] = x * 2;            /* Write c[i] */
        c[i] = z + 1;            /* Write c[i] again - WAW */
        
        /* ============================================
         * DEPENDENCY GROUP 4: Control flow dependencies
         * ============================================ */
        /* Conditional creates basic block boundaries */
        if (i % 2 == 0) {
            /* Flow dependency across basic blocks */
            y = b[i] + 1;        /* Read b[i] */
            b[i] = x + y;        /* Write b[i] - RAW: y -> b[i] */
            
            /* Anti dependency in conditional block */
            z = a[i];            /* Read a[i] */
            a[i] = z * 3;        /* Write a[i] - WAR: z = a[i] -> a[i] = ... */
        } else {
            /* Different dependency pattern in else branch */
            y = c[i] - 1;        /* Read c[i] */
            c[i] = y / 2;        /* Write c[i] - RAW: y -> c[i] */
            
            /* Output dependency in else branch */
            b[i] = x + 5;        /* Write b[i] */
            b[i] = z - 3;        /* Write b[i] again - WAW */
        }
        
        /* ============================================
         * DEPENDENCY GROUP 5: Register dependencies
         * ============================================ */
        /* Chain of register operations */
        int temp1 = x + y;       /* REG_DEP: x,y -> temp1 */
        int temp2 = temp1 * z;   /* REG_DEP: temp1,z -> temp2 */
        x = temp2 % 7;           /* REG_DEP: temp2 -> x */
        
        /* ============================================
         * DEPENDENCY GROUP 6: Nested loop dependencies
         * ============================================ */
        /* Inner loop creates additional dependencies */
        for (j = 0; j < 4; j++) {
            /* Flow dependency in inner loop */
            int inner_temp = a[i] + j;  /* RAW: a[i] -> inner_temp */
            a[i] = inner_temp - 1;      /* RAW: inner_temp -> a[i] */
            
            /* Anti dependency in inner loop */
            int read_val = b[i];        /* Read b[i] */
            b[i] = read_val + j;        /* Write b[i] - WAR */
        }
        
        /* ============================================
         * DEPENDENCY GROUP 7: Complex recurrence
         * ============================================ */
        /* Multi-distance loop-carried dependency */
        if (i >= 3) {
            /* Flow dependency with distance = 3 */
            c[i] = c[i - 3] * 2 + a[i - 2];  /* RAW: c[i-3], a[i-2] -> c[i] */
        }
    }
    
    /* ============================================
     * FINAL REDUCTION TO PREVENT DEAD CODE ELIMINATION
     * ============================================ */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use the result to prevent optimization */
    if (sum > 0) {
        return 0;  /* Success */
    } else {
        return 1;  /* Should never happen */
    }
}
