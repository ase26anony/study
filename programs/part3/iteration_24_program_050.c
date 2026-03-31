/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x, y, z, sum = 0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    // Complex loop with multiple dependency types
    for (int i = 1; i < SIZE; i++) {
        // Flow (RAW) dependency - memory to memory
        a[i] = b[i] + c[i];
        
        // Anti (WAR) dependency - memory to register then register to memory
        x = a[i];           // Read a[i]
        a[i] = x * 2;       // Write a[i] - WAR with previous read
        
        // Output (WAW) dependency
        a[i] = a[i] + 1;    // Write a[i] again - WAW with previous write
        
        // Loop-carried flow dependency (distance = 1)
        if (i > 0) {
            // Flow dependency across iterations
            y = a[i-1] + b[i];  // Read a[i-1] from previous iteration
            a[i] = y + c[i];    // Write a[i] - creates loop-carried edge
        }
        
        // More complex dependencies with conditional
        if (i % 2 == 0) {
            // Flow dependency within same iteration
            z = a[i] * 3;       // Read a[i]
            b[i] = z + i;       // Write b[i]
            
            // Anti dependency
            x = b[i];           // Read b[i]
            b[i] = x / 2;       // Write b[i] - WAR
            
            // Output dependency
            b[i] = b[i] + 5;    // Write b[i] again - WAW
        } else {
            // Different dependency pattern in else branch
            z = c[i] + a[i];    // Read both c[i] and a[i]
            c[i] = z - i;       // Write c[i]
            
            // Chain of dependencies
            x = c[i];
            y = x * 2;
            z = y + a[i];
            a[i] = z % 100;
        }
        
        // Register dependencies
        int r1 = a[i];
        int r2 = r1 + b[i];     // REG_DEP flow
        int r3 = r2 * c[i];
        r1 = r3 - i;            // REG_DEP anti (WAR on r1)
        r2 = r1 / 2;            // REG_DEP flow
        r3 = r2 + r1;           // REG_DEP flow
        a[i] = r3;
    }
    
    // Nested loop with different dependency pattern
    for (int i = 0; i < SIZE/2; i++) {
        for (int j = 1; j < SIZE/2; j++) {
            // Cross-iteration dependencies in both loops
            a[i*2 + j] = a[i*2 + j-1] + b[j];
            
            // Anti dependency in inner loop
            x = b[j];
            b[j] = a[i*2 + j] + x;
            
            // Output dependency
            c[j] = x * 2;
            c[j] = c[j] + i;
        }
    }
    
    // Reduction to prevent dead code elimination
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    // Use result to prevent optimization
    return sum % 1000;
}
