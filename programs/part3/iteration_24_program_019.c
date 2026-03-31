/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define N 256
#define M 128

int main() {
    int a[N], b[N], c[N];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < N; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    /* Complex loop nest with various dependencies */
    for (int i = 1; i < N; i++) {
        /* Loop-carried flow dependency (RAW) - distance = 1 */
        a[i] = a[i-1] + b[i];  // Flow dep on previous iteration
        
        /* Anti dependency (WAR) within same iteration */
        int temp = a[i];       // Read a[i]
        a[i] = temp + c[i];    // Write a[i] - anti dep with previous read
        
        /* Output dependency (WAW) */
        if (i % 2 == 0) {
            a[i] = a[i] * 2;   // Write a[i] again - output dep
        }
        
        /* Memory vs register dependencies mixed */
        x = y + z;             // Register flow dep
        y = x * 2;             // Register flow dep
        z = y - 1;             // Register flow dep
        
        /* Conditional creates basic block boundaries */
        if (i > M) {
            /* Additional dependencies in conditional block */
            b[i] = b[i-2] + a[i];  // Flow dep with distance 2
            c[i] = c[i] + x;       // Anti dep on x
            x = c[i] * 3;          // Flow dep on c[i]
        } else {
            /* Different path with its own dependencies */
            b[i] = b[i] * 2;       // Output dep on b[i]
            c[i] = b[i] + y;       // Flow dep on b[i], anti on y
            y = z + 1;             // Flow dep on z
        }
        
        /* Nested loop for additional complexity */
        for (int j = 0; j < 4; j++) {
            /* Cross-iteration dependencies in inner loop */
            a[i] = a[i] + j;       // Output dep on a[i]
            sum += a[i];           // Flow dep on a[i]
        }
    }
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", sum);
    
    /* Additional reduction to ensure side effects */
    int final_check = 0;
    for (int i = 0; i < N; i++) {
        final_check += a[i] + b[i] + c[i];
    }
    printf("Final check: %d\n", final_check);
    
    return 0;
}
