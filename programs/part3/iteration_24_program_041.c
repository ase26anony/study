/* ddg_test.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>

#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 1, y = 2, z = 3;
    int sum = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        b[i] = i;
        c[i] = i * 2;
    }
    
    /* Complex loop with multiple dependency types */
    for (int i = 0; i < SIZE; i++) {
        /* Flow (RAW) dependency - memory */
        a[i] = b[i] + c[i];      /* Source for next operation */
        
        /* Flow (RAW) dependency - register */
        x = a[i] * 2;            /* Depends on a[i] */
        
        /* Anti (WAR) dependency */
        y = a[i];                /* Read a[i] before overwrite */
        a[i] = x + y;            /* Overwrites a[i] - anti-dep with line above */
        
        /* Output (WAW) dependency */
        z = a[i] + 1;            /* Just to use a[i] */
        a[i] = z * 3;            /* Second write to a[i] - output dep */
        
        /* Loop-carried dependency (distance = 1) */
        if (i > 0) {
            /* Flow dependency across iterations */
            b[i] = b[i-1] + a[i];  /* Depends on b[i-1] from prev iteration */
        }
        
        /* Control flow creates basic block boundaries */
        if (i % 2 == 0) {
            /* Different operations in different basic blocks */
            c[i] = a[i] - b[i];    /* More dependencies */
        } else {
            c[i] = a[i] + b[i];    /* Alternative path */
        }
        
        /* Another anti dependency (WAR) */
        int temp = a[i];          /* Read a[i] */
        a[i] = temp + c[i];       /* Write a[i] again */
    }
    
    /* Second loop with different recurrence pattern */
    for (int i = 1; i < SIZE; i++) {
        /* Loop-carried flow dependency with distance 2 */
        if (i >= 2) {
            a[i] = a[i-2] * 2 + b[i];  /* Distance = 2 recurrence */
        }
        
        /* Nested loop to create more complex DDG */
        for (int j = 0; j < 4; j++) {
            /* Register dependencies in inner loop */
            x = x + a[i] + j;
            y = y * 2 - x;
            
            /* Anti dependency in inner loop */
            int old_x = x;
            x = y + j;
            y = old_x - 1;
        }
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
