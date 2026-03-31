#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

struct Data {
    int x[4];
    int y;
    int z;
};

// Force complex addressing by making these volatile
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int extra1 = 11;
volatile int extra2 = 13;
volatile int extra3 = 17;

// Prevent optimization of the complex access function
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    // Declare many local variables to consume registers
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = extra1;
    int v5 = extra2;
    int v6 = extra3;
    int v7 = n * 2;
    int v8 = n / 3;
    int v9 = n + 7;
    int v10 = n - 5;
    int v11 = v1 * v2;
    int v12 = v3 + v4;
    int v13 = v5 - v6;
    int v14 = v7 % 8;
    int v15 = v8 ^ v9;
    int v16 = v10 & 0xFF;
    int v17 = v11 | v12;
    int v18 = v13 << 2;
    int v19 = v14 >> 1;
    int v20 = v15 * v16;
    
    int sum = 0;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Complex index calculation using many live variables
        // This forces address computation to need multiple registers
        int idx = (i * v1 + v2 + v7 - v8) / v3;
        idx = (idx * v4 + v5) % (v6 + 1);
        
        // Ensure idx stays within bounds
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        // Complex load with struct field access and array indexing
        // This triggers RELOAD_FOR_INPUT_ADDRESS
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(idx * v9 + v10) % n].z;
        
        // More complex address calculations using live variables
        int idx2 = (i * v11 + v12) / (v13 + 1);
        idx2 = idx2 % n;
        
        // Complex store - triggers RELOAD_FOR_OUTPUT_ADDRESS
        arr[i].x[0] = val * v14 + v15;
        
        // Another store with different complex addressing
        // Triggers RELOAD_FOR_OUTADDR_ADDRESS
        arr[idx2].x[1] = val + v16;
        
        // Inline assembly to force address reloads
        // Triggers RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v17 + v18) % n].y));
        
        // More inline assembly with different addressing
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[idx].x[(v19 + i) % 4]));
        
        // Use all variables to keep them live
        val += v20;
        sum += val;
        
        // Modify some variables to prevent optimization
        v1 = (v1 + 1) % 5;
        v2 = (v2 + i) % 7;
        v3 = (v3 + 2) % 3;
    }
    
    return sum;
}

// Another noinline function to create more reload contexts
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 3 + j * 7) % 100;
        }
        arr[i].y = (i * 5) % 100;
        arr[i].z = (i * 11) % 100;
    }
}

int main() {
    // Use volatile to prevent constant propagation
    volatile int N = ARRAY_SIZE;
    
    // Allocate array dynamically to prevent stack optimization
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    // Initialize with pattern
    init_array(arr, N);
    
    // Perform complex accesses
    int result = complex_access_loop(arr, N);
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z;
    }
    
    // Use result and checksum
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
