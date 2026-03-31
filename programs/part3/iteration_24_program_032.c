/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x, y, z, sum = 0;
    int i, j;
    
    // Initialize arrays
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    // Complex loop nest with various dependency types
    for (i = 1; i < SIZE - 1; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i-1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        // Anti dependency (WAR) within same iteration
        x = a[i];              // Reads a[i]
        a[i] = x + c[i];       // Writes a[i] - anti dependency with previous read
        
        // Output dependency (WAW)
        y = b[i] * 2;          // Intermediate computation
        b[i] = y + 1;          // WAW: writes b[i] (though compiler might optimize)
        
        // Control flow creates basic block boundaries
        if (i % 3 == 0) {
            // Additional flow dependency in conditional block
            z = a[i] * 3;
            c[i] = z + b[i];   // Flow: reads a[i] and b[i], writes c[i]
            
            // Anti dependency across basic blocks
            x = c[i-1];        // Reads c[i-1]
        } else {
            // Different dependency pattern in else block
            c[i] = a[i] + x;   // Flow: reads a[i], writes c[i]
            
            // Output dependency in else block
            y = a[i] * 2;
            a[i] = y - 1;      // WAW: writes a[i] again
        }
        
        // Loop-carried anti dependency with distance 1
        b[i+1] = a[i] + 2;     // WAR: reads a[i], writes b[i+1] (anti across iterations)
        
        // Complex recurrence with multiple dependencies
        for (j = 0; j < 4; j++) {
            // Nested loop creates additional DDG complexity
            int temp = a[i] + j;
            c[i] = c[i] + temp; // Flow: reads c[i], writes c[i] (accumulator)
            
            // Register dependencies
            x = x + temp;       // Flow on scalar x
            y = y * 2;          // Independent operation
        }
    }
    
    // Final reduction to prevent dead code elimination
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    // Use result to prevent optimization
    if (sum > 0) {
        return 0;
    } else {
        return 1;
    }
}
