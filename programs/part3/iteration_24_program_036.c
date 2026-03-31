/* ddg_coverage.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O3 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_coverage.c -o ddg_coverage
 */

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int i, j, temp, sum = 0;
    
    // Initialize arrays
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    // Complex loop nest with multiple dependency types
    for (i = 1; i < SIZE - 1; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i-1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        // Anti dependency (WAR) within same iteration
        temp = a[i];           // Reads a[i]
        a[i] = c[i] * 3;       // Writes a[i] - anti dep with previous read
        
        // Output dependency (WAW)
        a[i] = temp + b[i];    // Writes a[i] again - output dep with previous write
        
        // Control flow creates multiple basic blocks
        if (i % 2 == 0) {
            // More dependencies in this branch
            b[i] = a[i] + c[i];  // Flow dep: reads a[i], writes b[i]
            c[i] = b[i-1] * 2;   // Flow dep with loop-carried distance 1
        } else {
            // Different dependencies in else branch
            c[i] = a[i-2] + b[i]; // Flow dep with distance 2
            a[i] = c[i] / 2;      // Flow dep and output dep
        }
        
        // Nested loop for additional complexity
        for (j = 0; j < 4; j++) {
            // Register and memory dependencies mixed
            int reg1 = a[i] + j;      // Register dependency
            int reg2 = reg1 * b[i];   // Register flow dep
            c[i] += reg2;            // Memory anti/output dep
            a[i] = reg2 - c[i];      // Memory flow/anti dep
        }
        
        // Cross-iteration dependency with varying distances
        if (i > 2) {
            b[i] = b[i-1] + b[i-2] + b[i-3];  // Multiple loop-carried deps
        }
    }
    
    // Final reduction to prevent dead code elimination
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    // Use result to prevent optimization
    if (sum > 0) {
        return 0;
    }
    return 1;
}
