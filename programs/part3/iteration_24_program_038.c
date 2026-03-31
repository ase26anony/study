/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    // Initialize arrays
    for (int i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    /* 
     * Complex loop with multiple dependency types to trigger DDG edge creation
     * This loop is designed to create:
     * 1. Flow dependencies (RAW)
     * 2. Anti dependencies (WAR)  
     * 3. Output dependencies (WAW)
     * 4. Loop-carried dependencies
     */
    for (int i = 1; i < SIZE; i++) {
        // Output dependency (WAW) - same array element written twice
        a[i] = b[i] + x;           // Write 1 to a[i]
        a[i] = a[i] * 2;           // Write 2 to a[i] - WAW with previous
        
        // Flow dependency (RAW) - read after write
        int temp = a[i];           // Read a[i] - RAW dependency
        
        // Anti dependency (WAR) - write after read
        x = temp + y;              // Write to x - WAR with previous read of x
        
        // Loop-carried flow dependency (distance = 1)
        c[i] = c[i-1] + z;         // Read c[i-1], write c[i]
        
        // Control flow to create basic block boundaries
        if (i % 2 == 0) {
            // Additional flow dependency in conditional block
            y = a[i] + c[i];       // Read a[i] and c[i]
            
            // Anti dependency in conditional block
            z = y * 3;             // Write to z - WAR with previous read of z
        } else {
            // Different dependency pattern in else block
            z = a[i] - b[i];       // Different computation
            
            // Output dependency in else block
            b[i] = z + i;          // Write to b[i] - WAW with initialization
        }
        
        // Another flow dependency after control flow
        temp = x + y + z;          // Multiple reads
        
        // Register pressure to force spill/reload
        int r1 = temp * 2;
        int r2 = r1 + a[i];
        int r3 = r2 - b[i];
        int r4 = r3 * c[i];
        
        // Final assignment with all dependency types
        a[i] = r4;                 // WAW with earlier a[i] writes
    }
    
    // Second loop with different pattern for more DDG edges
    for (int i = SIZE - 1; i > 0; i--) {
        // Reverse loop-carried dependency
        if (i < SIZE - 1) {
            b[i] = b[i+1] + a[i];  // Flow dependency across iterations
        }
        
        // Complex expression with multiple dependencies
        c[i] = (a[i] * b[i]) + (c[i] / 2);  // RAW on a[i], b[i]; WAR on c[i]
        
        // Nested loop to create more complex DDG
        for (int j = 0; j < 4; j++) {
            // Inner loop dependencies
            x = x + a[i] + j;      // Loop-carried on x
            y = y * 2 - b[i];      // Loop-carried on y
        }
    }
    
    // Reduction to prevent dead code elimination
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    // Use results to prevent optimization
    printf("Result: %d\n", sum + x + y + z);
    
    return 0;
}
