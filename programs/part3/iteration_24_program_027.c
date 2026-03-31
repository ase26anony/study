/* Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
    /* Declare arrays and scalars to create various dependencies */
    int a[N], b[N], c[N];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = 0;
    }
    
    /* 
     * Complex loop nest with multiple dependency types
     * This should create a rich DDG with various edge types
     */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i - 1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        /* Anti dependency (WAR) */
        x = a[i];                 // Read a[i]
        a[i] = y + i;             // Write a[i] - anti dep with previous read
        
        /* Output dependency (WAW) */
        c[i] = x * 2;             // Write c[i]
        if (i % 3 == 0) {         // Control flow creates basic block boundary
            /* Different basic block with more dependencies */
            c[i] = y + 1;         // Write c[i] again - output dep within same iteration
            y = c[i] + z;         // Flow dep: c[i] -> y
        } else {
            /* Alternative path with register dependencies */
            z = x + y;            // Register flow dep
            y = z * 2;            // Register flow dep
        }
        
        /* Memory anti dependency across arrays */
        b[i] = a[i] + c[i];       // Read a[i], c[i]; write b[i]
        
        /* Nested loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Recurrence with different distance */
            if (j > 0) {
                c[j % N] = c[(j - 1) % N] + 1;  // Loop-carried flow dep in inner loop
            }
            /* Mix of memory and register ops */
            x = a[j % N] + b[j % N];            // Memory reads
            y = x + j;                          // Register op
        }
        
        /* Another output dependency */
        a[i] = z + i;                          // Write a[i] again
    }
    
    /* Final computation to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to ensure side effects */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
