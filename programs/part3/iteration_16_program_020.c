#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 20

struct Data {
    int x[4];
    int y;
};

// Force complex addressing that can't be optimized away
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int extra) {
    // Declare many local variables to consume registers
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra + 4;
    int v5 = v1 * 2;
    int v6 = v2 * 2;
    int v7 = v3 * 2;
    int v8 = v4 * 2;
    int v9 = v1 + v2;
    int v10 = v3 + v4;
    int v11 = v5 + v6;
    int v12 = v7 + v8;
    int v13 = v9 * 3;
    int v14 = v10 * 3;
    int v15 = v11 * 3;
    int v16 = v12 * 3;
    int v17 = v13 + v14;
    int v18 = v15 + v16;
    int v19 = v17 * 2;
    int v20 = v18 * 2;
    
    int sum = 0;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Complex index calculation - forces address computation
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int idx = (i * v1 + v2) / v3;
        
        // Prevent loop-invariant code motion
        idx = idx + (i % v4);
        
        // Complex load with struct field access
        // Mixes array index and struct field - forces complex addressing
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        // Use many live variables in computation
        val = val * v5 + v6 - v7 + v8 * v9 - v10;
        
        // Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        // Nested array access with computation
        arr[i].x[(i + v11) % 4] = val * v12 + v13;
        
        // Another complex store with different addressing
        // Should trigger RELOAD_FOR_OUTADDR_ADDRESS
        arr[(i * v14 + v15) / v16].y = val + v17;
        
        // Inline assembly to force specific reload types
        // RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v18 + v19) / v20].x[0]));
        
        // More inline assembly with different addressing
        // Should trigger RELOAD_FOR_OTHER_ADDRESS
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[i].x[(v1 + v2) % 4]));
        
        // Keep variables live through the loop
        v1 = v1 + (i & 1);
        v2 = v2 + (i & 2);
        v3 = v3 + (i & 3);
        v4 = v4 + (i & 4);
        
        sum += val;
    }
    
    // Use all variables in final computation to keep them live
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

// Another noinline function to create more reload contexts
__attribute__((noinline, noipa))
void complex_store_operation(struct Data *arr, int idx, int val, 
                            volatile int a, volatile int b, volatile int c) {
    // Complex addressing in store
    arr[(idx * a + b) / c].x[(val + a) % 4] = val * b + c;
    
    // Inline assembly with memory constraint
    asm volatile("# Store address reload %0" 
                 : "=m" (arr[idx].y) 
                 : "r" (val));
}

int main() {
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 5;
    
    // Allocate and initialize array
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    // Initialize with pattern
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 11) % 100;
    }
    
    // Call complex access function
    int result = complex_access_loop(arr, N, stride, offset, scale, extra);
    
    // Additional complex store operations
    for (int i = 0; i < 100; ++i) {
        complex_store_operation(arr, i, result + i, 
                               stride, offset, scale);
    }
    
    // Compute checksum
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].y;
        for (int j = 1; j < 4; ++j) {
            checksum += arr[i].x[j];
        }
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
