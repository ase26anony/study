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
        c[i] = 0;
    }
    
    /* Complex loop nest with various dependencies */
    for (i = 1; i < N - 1; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i - 1] + b[i];  // Flow dep: a[i-1] -> a[i]
        
        /* Anti dependency (WAR) within same iteration */
        x = a[i];                // Read a[i]
        a[i] = y + i;            // Write a[i] - anti dep with previous read
        
        /* Output dependency (WAW) */
        c[i] = x * 2;            // Write c[i]
        if (i % 3 == 0) {
            /* Control flow creates basic block boundary */
            c[i] = y + 5;        // Another write to c[i] - output dep
            y = c[i] + 1;        // Flow dep: c[i] -> y
        } else {
            z = x + y;           // Register dependencies
        }
        
        /* Nested loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Memory and register mix */
            int temp = b[j % N];
            
            /* Multiple dependencies in nested loop */
            if (j % 2 == 0) {
                /* Flow dependency across loops */
                b[(j + 1) % N] = temp + a[i % N];
                
                /* Anti dependency */
                int old_val = c[j % N];
                c[j % N] = temp + old_val;
                
                /* Register pressure */
                x = x + old_val;
                y = y - temp;
            } else {
                /* Different dependency pattern */
                c[j % N] = c[j % N] + 1;
                z = z + b[j % N];
            }
        }
        
        /* Cross-iteration output dependency with distance > 1 */
        if (i > 10) {
            a[i - 10] = z;       // WAW with iteration i-10
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use results to ensure side effects */
    printf("Result: %d\n", sum + x + y + z);
    
    return 0;
}
