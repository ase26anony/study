#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

struct Data {
    int x[4];
    int y;
};

// Prevent optimization of complex addressing
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int extra1,
                       volatile int extra2, volatile int extra3) {
    // Declare many local variables to consume registers
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra1 + 4;
    int v5 = extra2 + 5;
    int v6 = extra3 + 6;
    int v7 = v1 * 2;
    int v8 = v2 * 2;
    int v9 = v3 * 2;
    int v10 = v4 * 2;
    int v11 = v5 * 2;
    int v12 = v6 * 2;
    int v13 = v7 + v8;
    int v14 = v9 + v10;
    int v15 = v11 + v12;
    int v16 = v13 * 3;
    int v17 = v14 * 3;
    int v18 = v15 * 3;
    int v19 = v16 + v17;
    int v20 = v18 + v19;
    
    int checksum = 0;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Complex index calculation - forces address computation in steps
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int idx = (i * v1 + v2) / v3;
        
        // Prevent hoisting of address calculations
        volatile int temp_idx = idx;
        idx = temp_idx;
        
        // Complex load with struct field access - multiple address components
        // Should trigger RELOAD_FOR_INPUT_ADDRESS and RELOAD_FOR_INPADDR_ADDRESS
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        // Modify val using many live variables to keep them alive
        val = val * v4 + v5 - v6 + v7 - v8 + v9 - v10 + v11 - v12;
        
        // Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        // and RELOAD_FOR_OUTADDR_ADDRESS
        arr[i].x[0] = val * v13 + v14;
        
        // Another complex store with different addressing
        // Should trigger different reload types
        arr[(i * v15 + v16) / v17].x[1] = val + v18;
        
        // Inline assembly to force address reloads
        // Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v19 + v20) / v1].y));
        
        // More inline assembly with different addressing
        // Should trigger RELOAD_FOR_OTHER_ADDRESS
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[idx].x[2]));
        
        // Keep checksum to prevent dead code elimination
        checksum += arr[i].x[0] + arr[i].x[1];
    }
    
    // Use all variables in final calculation to keep them live
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return checksum;
}

__attribute__((noinline, noipa))
void initialize_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 3) % 100;
        }
        arr[i].y = (i * 11) % 100;
    }
}

int main() {
    // Use volatile to prevent constant propagation
    volatile int N = ARRAY_SIZE;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra1 = 5;
    volatile int extra2 = 11;
    volatile int extra3 = 17;
    
    // Allocate array on heap to avoid stack overflow
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pattern
    initialize_array(arr, N);
    
    // Perform complex accesses
    int result = complex_access_loop(arr, N, stride, offset, scale, 
                                     extra1, extra2, extra3);
    
    // Additional complex operations to increase reload pressure
    for (int i = 0; i < 10; ++i) {
        // More complex addressing in a separate loop
        int idx = (i * stride + offset) / scale;
        arr[idx].x[3] = result + i;
        
        // Inline assembly with output addressing
        int temp;
        asm volatile("movl %1, %0\n\t"
                     : "=r" (temp)
                     : "m" (arr[(i * extra1 + extra2) / extra3].y));
        result += temp;
    }
    
    printf("Result: %d\n", result);
    
    free(arr);
    return 0;
}
