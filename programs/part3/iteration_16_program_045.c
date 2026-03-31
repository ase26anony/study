#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 20

struct Data {
    int x[4];
    int y;
};

// Prevent optimization of complex addressing calculations
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int extra) {
    // Declare many local variables to consume registers
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = extra * 2;
    int v9 = stride + offset;
    int v10 = scale + extra;
    int v11 = stride * offset;
    int v12 = scale * extra;
    int v13 = v1 + v2;
    int v14 = v3 + v4;
    int v15 = v5 + v6;
    int v16 = v7 + v8;
    int v17 = v9 + v10;
    int v18 = v11 + v12;
    int v19 = v13 + v14;
    int v20 = v15 + v16;
    
    int sum = 0;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Force complex addressing mode for input (LOAD)
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int idx1 = (i * v1 + v2) / v3;
        idx1 = idx1 % (n - 1);
        if (idx1 < 0) idx1 = -idx1;
        
        // More complex index calculation using many live variables
        int idx2 = ((i * v4 + v5) * v6 + v7) / v8;
        idx2 = idx2 % (n - 1);
        if (idx2 < 0) idx2 = -idx2;
        
        // Complex addressing in load - should trigger RELOAD_FOR_INPUT_ADDRESS
        // and possibly RELOAD_FOR_INPADDR_ADDRESS
        int val = arr[idx1].x[i % 4] + arr[i-1].y;
        val += arr[idx2].x[(i+1) % 4];
        
        // Use all live variables in computation to keep them alive
        val += v9 + v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        // Complex addressing in store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        // and possibly RELOAD_FOR_OUTADDR_ADDRESS
        int store_idx = (i * v3 + v4) / v1;
        store_idx = store_idx % n;
        if (store_idx < 0) store_idx = -store_idx;
        
        arr[store_idx].x[0] = val * v2 + v1;
        
        // Force address reload with inline assembly
        // Should trigger RELOAD_FOR_OPERAND_ADDRESS or RELOAD_FOR_OPADDR_ADDR
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[store_idx].y)
                     : "memory");
        
        // Another inline assembly with different addressing
        // Should trigger RELOAD_FOR_OTHER_ADDRESS
        int asm_idx = (i * v5 + v6) / v7;
        asm_idx = asm_idx % n;
        if (asm_idx < 0) asm_idx = -asm_idx;
        
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[asm_idx].x[2])
                     : "memory");
        
        sum += arr[store_idx].x[0];
    }
    
    // Use all variables in return to keep them live
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

// Another noinline function to force more reload contexts
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23) % 100;
    }
}

int main() {
    volatile int N = 1024;  // volatile to prevent constant propagation
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 11;
    
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    init_array(arr, N);
    
    int result = complex_access_loop(arr, N, stride, offset, scale, extra);
    
    printf("Result: %d\n", result);
    
    // Compute checksum to verify execution
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].y;
        for (int j = 0; j < 4; ++j) {
            checksum += arr[i].x[j];
        }
    }
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
