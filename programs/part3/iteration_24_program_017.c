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
     * Complex loop nest with multiple dependency types
     * This should create various DDG edges
     */
    for (i = 1; i < N - 1; i++) {
        // Loop-carried flow dependency (RAW) with distance 1
        a[i] = a[i - 1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        // Anti dependency (WAR) within same iteration
        x = a[i];                // Read a[i]
        a[i] = y + i;            // Write a[i] - anti dep with previous read
        
        // Output dependency (WAW) on a[i]
        if (i % 2 == 0) {
            a[i] = x * 2;        // Another write to a[i] - output dep
            z = b[i] + 1;        // Independent operation
        } else {
            a[i] = y + 3;        // Alternative write - control flow creates BB boundary
        }
        
        // Nested loop for additional complexity
        for (j = 0; j < M; j++) {
            // Memory and register mix with loop-carried dependency
            c[j] = c[j] + a[i];  // Flow dep on c[j] across j iterations
            
            // Anti dependency in nested loop
            int temp = c[j];      // Read c[j]
            c[j] = temp + b[i];   // Write c[j] - anti dep
            
            // Scalar operation creating register dependencies
            y = y + temp;         // Flow dep on y
            x = y * 2;            // Flow dep on x from y
        }
        
        // Control flow creates basic block boundaries
        if (a[i] > 100) {
            // More dependencies in conditional block
            b[i] = a[i] - 50;     // Flow dep on a[i]
            z = z + b[i];         // Flow dep on z
        } else {
            // Alternative path with different deps
            b[i] = a[i] + 50;     // Flow dep on a[i]
            y = y + b[i];         // Flow dep on y
        }
        
        // Output dependency across array elements
        a[i + 1] = x + y;         // Flow deps on x, y; sets up next iteration
    }
    
    // Final reduction to prevent dead code elimination
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i % M];
    }
    
    // Use result to prevent optimization
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
