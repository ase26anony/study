/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define N 256
#define M 128

int main() {
    int a[N], b[N], c[N];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
    }
    
    /* Complex loop nest with various dependency types */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        /* Anti dependency (WAR) */
        x = a[i];              // Read a[i]
        a[i] = c[i] * 2;       // Write a[i] - anti dep with previous read
        
        /* Output dependency (WAW) */
        y = b[i] + x;          // Intermediate computation
        b[i] = y * 3;          // Write b[i]
        b[i] = b[i] + 1;       // Another write to b[i] - output dep
        
        /* Control flow to create basic block boundaries */
        if (i % 2 == 0) {
            /* Different dependency pattern in this branch */
            z = a[i] + b[i];   // Flow dep on both a[i] and b[i]
            c[i] = z * 2;      // Flow dep: z -> c[i]
        } else {
            /* Alternative pattern with anti dependency */
            int temp = c[i];   // Read c[i]
            c[i] = a[i] * b[i]; // Write c[i] - anti dep
            z = temp + 1;      // Use previous read
        }
        
        /* Nested inner loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Register and memory dependencies mixed */
            x = x + a[i] + j;  // Flow dep on x (register), anti on a[i]
            y = b[i] * j;      // Flow dep on b[i]
            
            /* Output dependency in inner loop */
            z = x + y;
            z = z * 2;         // Output dep on z
            
            /* Memory dependency with distance in inner loop */
            if (j > 0) {
                c[i] = c[i] + a[i] * (j-1); // Flow dep on c[i] from previous iteration
            }
        }
        
        /* Cross-iteration dependency with variable distance */
        if (i > 10) {
            a[i] = a[i-10] + a[i]; // Flow dep with distance 10
        }
    }
    
    /* Reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
