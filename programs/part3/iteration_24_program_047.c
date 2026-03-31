/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define N 256
#define M 128

int main() {
    int a[N], b[N], c[N];
    int x = 0, y = 1, z = 2;
    int sum = 0;
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* 
     * Complex loop with multiple dependency types to trigger DDG edge creation
     * This should generate flow, anti, and output dependencies
     */
    for (int i = 1; i < N - 1; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i-1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        // Anti dependency (WAR) within same iteration
        x = a[i];              // Reads a[i]
        a[i] = y + c[i];       // Writes a[i] - anti dependency with previous read
        
        // Output dependency (WAW) - multiple writes to same location
        if (i % 2 == 0) {
            a[i] = z * 2;      // Another write to a[i] - output dependency
        }
        
        // More complex dependencies with conditional
        if (i > M) {
            // Nested dependencies
            b[i] = a[i] + x;   // Flow dep on a[i] and anti dep on x
            x = b[i-1];        // Loop-carried anti dep on b
        } else {
            // Alternative path with different deps
            c[i] = c[i-1] + 1; // Loop-carried flow dep on c
            y = c[i];          // Anti dep on c[i]
        }
        
        // Cross-iteration register dependencies
        z = x + y;             // Depends on x and y from current iteration
        sum += z;              // Reduction to prevent elimination
    }
    
    // Additional loop with different pattern
    for (int i = 0; i < N; i += 2) {
        // Memory and register mix
        int temp = a[i];
        a[i] = b[i] + temp;    // Flow and anti mix
        b[i] = temp - a[i+1];  // More complex deps
        sum += a[i] + b[i];
    }
    
    // Prevent dead code elimination
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
