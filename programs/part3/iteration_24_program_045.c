/* ddg_edge_coverage.c
 * 
 * This program creates complex data dependencies to trigger DDG edge creation
 * in GCC's ddg.cc module, specifically targeting lines 749-757 in create_ddg_dep_edge.
 * 
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_edge_coverage.c -o ddg_test
 * 
 * Alternative flags: -O3 -fschedule-insns -fschedule-insns2
 * For ARM: -O3 -fmodulo-sched -march=armv8-a -mtune=cortex-a57
 */

#define SIZE 256

int main() {
    /* Declare arrays and scalars to create various dependency types */
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = SIZE - i;
    }
    
    /* ============================================
     * LOOP 1: Complex dependency patterns
     * ============================================ */
    for (i = 0; i < SIZE; i++) {
        /* FLOW (RAW) dependency: Read b[i], write to a[i] */
        a[i] = b[i] + c[i];
        
        /* ANTI (WAR) dependency: Read a[i], then write to it */
        x = a[i];          /* Read a[i] */
        a[i] = x + y;      /* Write a[i] - creates anti-dependency with previous read */
        
        /* OUTPUT (WAW) dependency: Multiple writes to same location */
        y = i * 2;         /* Write y */
        y = y + 1;         /* Write y again - output dependency */
        
        /* Control flow to create multiple basic blocks */
        if (i % 2 == 0) {
            /* FLOW dependency across iterations (loop-carried) */
            if (i > 0) {
                a[i] = a[i] + a[i-1];  /* Distance = 1 */
            }
            
            /* REGISTER dependency */
            z = z + a[i];  /* Accumulator with loop-carried dependency */
        } else {
            /* Different dependency pattern in else branch */
            b[i] = b[i] + x;
            
            /* MEMORY dependency with anti-dependency */
            int temp = a[i];  /* Read a[i] */
            a[i] = temp * 2;  /* Write a[i] */
        }
    }
    
    /* ============================================
     * LOOP 2: Nested loops with more dependencies
     * ============================================ */
    for (i = 1; i < SIZE; i++) {
        for (j = 1; j < 8; j++) {  /* Small inner loop for modulo scheduling */
            /* Complex loop-carried dependencies */
            c[i] = c[i] + a[i-j] * b[i];
            
            /* Multiple register dependencies */
            x = y + z;
            y = x * 2;
            z = y - x;
            
            /* Output dependency on array */
            a[i] = a[i] + 1;
            a[i] = a[i] * 2;  /* Second write to a[i] */
        }
    }
    
    /* ============================================
     * LOOP 3: Reduction with dependencies
     * ============================================ */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        /* Flow dependency through sum (loop-carried) */
        sum = sum + a[i];
        
        /* Anti-dependency pattern */
        int old_val = b[i];
        b[i] = sum % 100;
        
        /* Use old_val to prevent dead code elimination */
        c[i] = c[i] + old_val;
    }
    
    /* Final computation to ensure side effects */
    int result = sum + x + y + z;
    
    /* Prevent optimization by using result */
    if (result > 1000000) {
        return 1;
    }
    
    return 0;
}
