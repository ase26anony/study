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
        /* FLOW (RAW) dependency: read b[i], write a[i] */
        a[i] = b[i] + x;
        
        /* Another FLOW dependency: read a[i], update x */
        x = a[i] * 2;
        
        /* Introduce control flow to create basic blocks */
        if (i % 2 == 0) {
            /* ANTI (WAR) dependency in true branch: read a[i], then write it */
            int temp = a[i];      /* Read */
            a[i] = y + z;         /* Write - creates anti-dependency */
            y = temp;
            
            /* OUTPUT (WAW) dependency: multiple writes to same location */
            a[i] = a[i] * 3;      /* First write */
            a[i] = a[i] + 1;      /* Second write to same location */
        } else {
            /* Different operations in false branch */
            z = a[i] - b[i];
        }
        
        /* LOOP-CARRIED dependency (distance = 1) */
        if (i > 0) {
            /* Flow dependency across iterations */
            c[i] = c[i-1] + a[i];  /* c[i] depends on c[i-1] from previous iteration */
        }
        
        /* Mixed register and memory dependencies */
        int reg1 = a[i];          /* REG_DEP */
        int reg2 = b[i];          /* REG_DEP */
        a[i] = reg1 + reg2;       /* MEM_DEP (writing to memory) */
        
        /* Another loop-carried dependency with distance > 1 */
        if (i >= 2) {
            b[i] = b[i-2] * 2;    /* Distance = 2 */
        }
    }
    
    /* Nested loops for additional complexity */
    for (int i = 1; i < SIZE/2; i++) {
        for (int j = 1; j < SIZE/4; j++) {
            /* Cross-iteration dependencies in nested loops */
            a[i*2 + j] = a[(i-1)*2 + j] + b[i*2 + (j-1)];
            
            /* Anti-dependency in inner loop */
            int temp_val = b[j];
            b[j] = a[i*2 + j];
            a[i*2 + j] = temp_val;
        }
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", sum);
    return sum;
}
