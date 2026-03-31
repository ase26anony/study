/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    // Declare arrays and scalars to create various dependency types
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    // Initialize arrays
    for (i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = SIZE - i;
    }
    
    // Main computation with complex dependencies
    for (i = 0; i < SIZE; i++) {
        // FLOW (RAW) dependency: a[i] depends on b[i] and c[i]
        a[i] = b[i] + c[i];
        
        // Another FLOW dependency: x depends on a[i]
        x = a[i] * 2;
        
        // ANTI (WAR) dependency: y reads a[i] before it's overwritten
        y = a[i] + x;
        
        // OUTPUT (WAW) dependency: a[i] is written twice
        a[i] = y / 3;
        
        // Introduce control flow to create basic block boundaries
        if (i % 2 == 0) {
            // More FLOW dependencies inside conditional
            z = a[i] + 5;
            
            // ANTI dependency across conditional boundary
            b[i] = z * 2;
        } else {
            // Different dependency pattern in else branch
            z = a[i] - 3;
            
            // OUTPUT dependency in else branch
            c[i] = z + 1;
        }
        
        // Loop-carried FLOW dependency (distance = 1)
        // This creates recurrence in the DDG
        if (i > 0) {
            a[i] = a[i] + a[i-1];
        }
    }
    
    // Nested loop with different dependency patterns
    for (i = 1; i < SIZE; i++) {
        for (j = 1; j < 8; j++) {
            // 2D loop-carried dependencies
            // Horizontal (inner loop) and vertical (outer loop) dependencies
            
            // Inner loop FLOW dependency (distance = 1 in j dimension)
            b[i] = b[i] + c[j];
            
            // Outer loop FLOW dependency (distance = 1 in i dimension)
            if (j == 1) {
                a[i] = a[i] + a[i-1];
            }
            
            // Mixed memory and register dependencies
            int temp = b[i] * j;
            c[j] = temp + i;
            
            // ANTI dependency in nested loop
            temp = c[j];
            c[j] = temp * 2;
        }
    }
    
    // Final reduction to prevent dead code elimination
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i % 8];
    }
    
    // Use the result to ensure computation isn't optimized away
    return sum % 100;
}
