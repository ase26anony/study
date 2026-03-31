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
    
    /* Nested loops with various dependency types */
    for (i = 1; i < SIZE - 1; i++) {
        /* Loop-carried flow dependency (RAW with distance 1) */
        a[i] = a[i-1] + b[i];  // Flow dep: reads a[i-1], writes a[i]
        
        /* Anti dependency (WAR) */
        x = c[i];              // Reads c[i]
        c[i] = a[i] * 2;       // Writes c[i] - anti-dep with previous read
        
        /* Output dependency (WAW) */
        b[i] = x + i;          // First write to b[i]
        if (i % 3 == 0) {      // Control flow creates basic block boundary
            /* Different basic block with more dependencies */
            b[i] = y * 3;      // Second write to b[i] - output dep
            y = a[i] + z;      // Flow dep on a[i]
        } else {
            /* Alternative path */
            z = b[i] + x;      // Flow dep on b[i], anti-dep on x
        }
        
        /* More complex pattern with register and memory mix */
        for (j = 0; j < 4; j++) {
            /* Inner loop creates additional dependencies */
            int temp = a[i] + j;      // Register flow dep on a[i]
            a[i] = temp - b[i];       // Flow+anti: reads b[i], writes a[i]
            c[i] = c[i] + temp;       // Flow dep on c[i] and temp
        }
        
        /* Cross-iteration anti dependency */
        int old_x = x;          // Save old x
        x = b[i] * 2;           // Write x
        if (i > 10) {
            a[i-10] = old_x;    // Anti-dep on x across iterations
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += a[i] + b[i] + c[i];
    }
    
    /* Use result to prevent optimization */
    return sum % 100;
}
