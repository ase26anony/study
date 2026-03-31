/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

int main() {
    int i, j;
    int a[N], b[N], c[N];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    /* Complex loop nest with various dependency types */
    for (i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) - distance = 1 */
        a[i] = a[i-1] + b[i];  // Flow dep on a[i-1]
        
        /* Anti dependency (WAR) within same iteration */
        x = c[i];              // Read c[i]
        c[i] = y + z;          // Write c[i] - anti dep on previous read
        
        /* Output dependency (WAW) */
        y = a[i] * 2;          // Write y
        if (i % 2 == 0) {      // Control flow creates basic block boundary
            /* Different basic block with more dependencies */
            y = x + i;         // Another write to y - output dep on previous y
            
            /* Memory and register mix */
            z = b[i] + c[i];   // Flow dep on b[i], c[i]
            b[i] = z * 3;      // Flow dep on z, output dep on b[i]
        } else {
            /* Alternative path with different dependencies */
            int temp = a[i] + c[i];  // Flow deps on a[i], c[i]
            b[i] = temp / 2;         // Flow dep on temp
            
            /* Create anti dependency across paths */
            x = b[i] + 1;            // Anti dep on b[i] from else path
        }
        
        /* Nested loop for additional complexity */
        for (j = 0; j < M; j++) {
            /* Recurrence in inner loop */
            if (j > 0) {
                c[j % N] = c[(j-1) % N] + 1;  // Inner loop-carried dep
            }
            
            /* Mixed dependencies in inner loop */
            int inner_temp = a[j % N];
            a[j % N] = b[j % N] + inner_temp;  // Anti + Flow deps
            b[j % N] = inner_temp * 2;         // Flow dep on inner_temp
        }
        
        /* Cross-iteration register dependencies */
        z = y + x;      // Flow deps on y, x (may have loop-carried components)
    }
    
    /* Final computation to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
