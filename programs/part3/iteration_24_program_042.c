/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars for various dependency types */
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0, w = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    /* ============================================
     * LOOP 1: Complex dependency patterns
     * ============================================ */
    for (i = 1; i < SIZE; i++) {
        /* FLOW (RAW) dependency: Read after Write */
        int temp = a[i-1] + b[i];  /* Read a[i-1] */
        a[i] = temp;               /* Write a[i] - creates loop-carried flow dep */
        
        /* ANTI (WAR) dependency: Write after Read */
        x = c[i];                  /* Read c[i] */
        if (i % 2 == 0) {         /* Control flow creates basic block boundary */
            /* Different basic block with anti dependency */
            c[i] = x + 1;         /* Write c[i] after reading it - anti dep */
        } else {
            /* Alternative path with output dependency */
            c[i] = y;             /* Write c[i] - potential output dep with else branch */
            y = i * 3;
        }
        
        /* OUTPUT (WAW) dependency: Write after Write */
        b[i] = z;                 /* First write to b[i] */
        z = z + a[i];             /* Update z */
        b[i] = z;                 /* Second write to b[i] - output dep */
        
        /* Another flow dependency chain */
        w = w + temp;
    }
    
    /* ============================================
     * LOOP 2: Nested loops with cross-iteration dependencies
     * ============================================ */
    for (i = 0; i < SIZE/2; i++) {
        for (j = 1; j < SIZE/2; j++) {
            /* Loop-carried flow dependency in inner loop */
            a[i*2 + j] = a[i*2 + j-1] + b[j];
            
            /* Anti dependency with conditional */
            int val = c[j];
            if (val > SIZE/2) {
                c[j] = val - 1;  /* Anti dep: read then write c[j] */
            }
            
            /* Register and memory mix */
            x = x + a[i*2 + j];
            y = y * 2 + 1;
        }
        
        /* Output dependency across outer loop iterations */
        b[i] = x;      /* First write */
        b[i] = y;      /* Second write - output dep */
    }
    
    /* ============================================
     * LOOP 3: Reduction with multiple dependency types
     * ============================================ */
    int sum = 0;
    int prod = 1;
    
    for (i = 0; i < SIZE; i++) {
        /* Flow: sum depends on previous sum */
        sum = sum + a[i];
        
        /* Anti: read a[i] before modifying it */
        int old_a = a[i];
        a[i] = (a[i] + b[i]) * c[i];
        
        /* Flow: prod depends on previous prod */
        prod = prod * (old_a + 1);
        
        /* Output: multiple writes to same memory */
        if (i % 3 == 0) {
            b[i] = sum;
        } else if (i % 3 == 1) {
            b[i] = prod;
        } else {
            b[i] = sum + prod;
        }
    }
    
    /* Final computation to prevent dead code elimination */
    int result = sum + prod + x + y + z + w + a[SIZE-1] + b[0] + c[SIZE/2];
    
    /* Use result to ensure side effects */
    return result % 256;  /* Prevent overflow, ensure compiler keeps computations */
}
