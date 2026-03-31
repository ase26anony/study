/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    /* Complex loop with various dependency patterns */
    for (int i = 1; i < SIZE; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i - 1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        /* Anti dependency (WAR) within same iteration */
        x = a[i];                // Reads a[i]
        a[i] = y + c[i];         // Writes a[i] - anti dep with previous read
        
        /* Output dependency (WAW) */
        y = x * 2;               // Writes y
        if (i % 2 == 0) {        // Control flow creates basic block boundary
            /* Memory anti dependency across arrays */
            z = b[i];            // Reads b[i]
            b[i] = z + 1;        // Writes b[i] - anti dep
            
            /* Flow dependency with register */
            int temp = z + x;    // Reads z and x
            c[i] = temp;         // Writes c[i]
            
            /* Output dependency in else branch */
            y = temp * 3;        // Another write to y - output dep
        } else {
            /* Different dependency pattern in else branch */
            c[i] = b[i] + a[i];  // Flow deps on b[i] and a[i]
            
            /* Register flow dependency chain */
            x = y + 1;           // Reads y
            y = x * 2;           // Reads x, writes y
        }
        
        /* Cross-iteration anti dependency with distance > 1 */
        if (i > 2) {
            b[i - 2] = a[i] + c[i - 1];  // Reads a[i], c[i-1]; writes b[i-2]
        }
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum);
    return sum;
}
