/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define N 256
#define M 128

int main() {
    int a[N], b[N], c[N];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    // Initialize arrays
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* 
     * Complex loop nest with various dependency patterns
     * This should create multiple DDG edges with different types
     */
    for (i = 1; i < N; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i - 1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        // Anti dependency (WAR) within same iteration
        x = a[i];                // Read a[i]
        a[i] = y + i;            // Write a[i] - anti dep with previous read
        
        // Output dependency (WAW)
        c[i] = x * 2;            // Write c[i]
        c[i] = c[i] + 1;         // Write c[i] again - output dep
        
        // Control flow creates basic block boundaries
        if (i % 2 == 0) {
            // Memory and register mix in conditional block
            y = b[i] * 3;        // Register operation
            b[i] = y + a[i];     // Memory write with flow dep from y
        } else {
            // Different dependency pattern in else block
            z = a[i] + b[i];     // Flow from a[i], b[i]
            a[i] = z / 2;        // Anti dep: z read, a[i] written
        }
        
        // Nested loop for additional complexity
        for (j = 0; j < M; j++) {
            // Cross-iteration dependency in inner loop
            if (j > 0) {
                c[j % N] = c[(j - 1) % N] + i;  // Flow dep with distance 1
            }
            
            // Anti dependency in inner loop
            int temp = b[j % N];  // Read
            b[j % N] = temp + j;  // Write - anti dep
            
            // Mix of operations
            x = x + temp;
        }
        
        // Output dependency across outer loop iterations
        y = x % 100;
    }
    
    // Additional dependency patterns in separate loop
    for (i = 0; i < N - 1; i++) {
        // True dependency chain
        int t1 = a[i] + b[i];
        int t2 = t1 * c[i];      // Flow dep: t1 -> t2
        int t3 = t2 - a[i + 1];  // Flow dep: t2 -> t3, a[i+1]
        a[i] = t3;               // Flow dep: t3 -> a[i]
        
        // Anti and output dependencies combined
        int old = b[i];          // Read b[i]
        b[i] = old + t3;         // Write b[i] - anti dep
        b[i] = b[i] * 2;         // Write b[i] again - output dep
    }
    
    // Prevent dead code elimination
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    // Use result to prevent optimization
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
