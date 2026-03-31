/* ddg_edge_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_edge_coverage.c -o ddg_test
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
    
    /* Main loop with complex dependency patterns */
    for (i = 0; i < SIZE; i++) {
        /* FLOW (RAW) dependency: a[i] depends on b[i] and c[i] */
        a[i] = b[i] + c[i];
        
        /* Another FLOW dependency: x depends on a[i] */
        x = a[i] * 2;
        
        /* ANTI (WAR) dependency: y reads a[i] before it's overwritten */
        y = a[i] + x;
        
        /* OUTPUT (WAW) dependency: a[i] is written twice */
        a[i] = y / 3;
        
        /* Introduce control flow to create basic block boundaries */
        if (i % 2 == 0) {
            /* FLOW dependency across basic blocks */
            z = a[i] + 5;
            
            /* ANTI dependency with scalar */
            x = z * 2;
        } else {
            /* Different dependency pattern in else branch */
            z = a[i] - 3;
            
            /* OUTPUT dependency on scalar */
            x = 10;
            x = z + 1;
        }
        
        /* Loop-carried FLOW dependency (distance = 1) */
        if (i > 0) {
            /* a[i] depends on a[i-1] from previous iteration */
            a[i] = a[i] + a[i-1];
        }
    }
    
    /* Nested loop with different dependency pattern */
    for (i = 1; i < SIZE; i++) {
        for (j = 1; j < 8; j++) {  /* Small inner loop for modulo scheduling */
            /* Loop-carried dependency in inner loop */
            b[i] = b[i] + c[j];
            
            /* Cross-iteration dependency with distance > 1 */
            if (j >= 2) {
                c[j] = c[j] + b[i-2];  /* Distance = 2 */
            }
        }
    }
    
    /* Reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    return sum % 100;
}
