/* ddg_test.c
 * Program designed to trigger DDG edge creation in GCC's ddg.cc
 * Compile with: gcc -O2 -fmodulo-sched -fmodulo-sched-allow-regmoves ddg_test.c -o ddg_test
 */

#include <stdio.h>

#define N 256
#define M 128

int main() {
    int a[N], b[N], c[N];
    int x, y, z, sum = 0;
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        b[i] = i;
        c[i] = N - i;
    }
    
    // Complex loop with multiple dependency types
    for (int i = 1; i < N; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i-1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        // Anti dependency (WAR) within same iteration
        x = a[i];              // Read a[i]
        a[i] = c[i] * 2;       // Write a[i] - anti dep with previous read
        
        // Output dependency (WAW)
        y = b[i] + x;          // Intermediate computation
        b[i] = y + i;          // Write b[i] - output dep if b[i] was written earlier
        
        // Control flow creates multiple basic blocks
        if (i % 3 == 0) {
            // Additional flow dependency in conditional block
            z = a[i] + b[i];
            c[i] = z * 3;      // Flow: z -> c[i]
            
            // Anti dependency across conditional boundary
            int temp = c[i-1]; // Read
            c[i-1] = temp + 1; // Write - anti dep
        } else if (i % 3 == 1) {
            // Different path with output dependency
            a[i] = b[i] * 4;   // Output dep: overwrites a[i] from earlier
            b[i] = a[i] / 2;   // Flow: a[i] -> b[i]
        } else {
            // Third path with register dependencies
            int r1 = a[i];
            int r2 = b[i];
            int r3 = r1 + r2;  // Register flow deps
            a[i] = r3;
            b[i] = r3 * 2;
        }
        
        // Cross-iteration anti dependency with distance > 0
        if (i > 2) {
            int old_val = c[i-2];  // Read from 2 iterations ago
            c[i] = old_val + a[i]; // Flow: old_val -> c[i]
        }
    }
    
    // Nested loop for additional complexity
    for (int i = 0; i < M; i++) {
        for (int j = 1; j < M; j++) {
            // 2D loop-carried dependencies
            a[i*M + j] = a[i*M + j-1] + b[j];  // Inner loop flow dep
            b[j] = a[i*M + j] - c[j];          // Flow + anti
            
            // Register pressure
            int reg1 = a[i*M + j];
            int reg2 = b[j];
            int reg3 = reg1 * reg2;
            int reg4 = reg3 + reg2;
            c[j] = reg4;
        }
    }
    
    // Prevent dead code elimination
    for (int i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum);
    return sum % 100;  // Ensure computation isn't optimized away
}
