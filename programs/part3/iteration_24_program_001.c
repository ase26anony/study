/* Complex dependency pattern generator for DDG edge coverage */
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
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < SIZE; i++) {
        /* FLOW (RAW) dependency: read after write */
        a[i] = b[i] + c[i];          /* Write a[i] */
        x = a[i] * 2;                /* Read a[i] - flow dep on line above */
        
        /* ANTI (WAR) dependency: write after read */
        y = a[i-1];                  /* Read a[i-1] */
        a[i-1] = z + i;              /* Write a[i-1] - anti dep on line above */
        
        /* OUTPUT (WAW) dependency: write after write */
        if (i % 3 == 0) {            /* Control flow creates basic block boundary */
            a[i] = x + y;            /* Write a[i] - output dep on first a[i] write */
            z = a[i] / 2;            /* Read a[i] - flow dep */
        } else {
            a[i] = y - x;            /* Alternative write - also output dep */
            z = a[i] * 3;            /* Read a[i] - flow dep */
        }
        
        /* Loop-carried dependency (distance = 1) */
        b[i] = b[i-1] + a[i];        /* Flow dep on b[i-1] from previous iteration */
        
        /* Another loop-carried dependency with distance > 1 */
        if (i >= 3) {
            c[i] = c[i-2] + b[i];    /* Distance = 2 flow dep */
        }
    }
    
    /* Prevent dead code elimination */
    for (int i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    return sum % 100;
}
