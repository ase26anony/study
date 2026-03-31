/* Complex dependency pattern generator for DDG edge coverage */
#define SIZE 256

int main() {
    int a[SIZE], b[SIZE], c[SIZE];
    int x = 0, y = 0, z = 0;
    int i, j;
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = SIZE - i;
        c[i] = i * 2;
    }
    
    /* Main loop with complex dependencies */
    for (i = 1; i < SIZE; i++) {
        /* Loop-carried flow dependency (RAW) with distance 1 */
        a[i] = a[i-1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        /* Anti dependency (WAR) within same iteration */
        x = a[i];              // Reads a[i]
        a[i] = y + c[i];       // Writes a[i] - anti dependency with previous read
        
        /* Output dependency (WAW) */
        y = x * 2;             // Writes y
        y = z + i;             // Writes y again - output dependency
        
        /* Control flow creates multiple basic blocks */
        if (i % 2 == 0) {
            /* Nested loop for additional complexity */
            for (j = 0; j < 4; j++) {
                /* More flow dependencies */
                b[i] = b[i] + c[j % SIZE];
                
                /* Register and memory mix */
                z = z + b[i];  // Register dependency
                c[j % SIZE] = z;  // Memory write
            }
        } else {
            /* Alternative path with different dependencies */
            int temp = b[i];
            b[i] = a[i] * temp;  // Flow: reads a[i], writes b[i]
            a[i] = temp - 1;     // Anti: reads temp, writes a[i]
        }
        
        /* Cross-iteration anti dependency */
        c[i] = x + y;           // Reads x, y (from current iteration)
        x = i * 3;              // Writes x - anti with next iteration's read
    }
    
    /* Additional loop nest for more DDG edges */
    for (i = 0; i < SIZE - 1; i++) {
        for (j = i + 1; j < SIZE; j++) {
            /* Complex memory dependencies */
            if (j % 3 == 0) {
                a[i] = a[i] + b[j];  // Flow across different arrays
                b[j] = a[i] - c[j];  // Anti: reads a[i] just written
            }
        }
    }
    
    /* Final reduction to prevent elimination */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent dead code elimination */
    return sum % 100;
}
