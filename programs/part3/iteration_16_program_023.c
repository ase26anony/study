#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

struct Data {
    int x[4];
    int y;
};

// Force complex addressing to remain
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int mod1, volatile int mod2) {
    // Many live variables to exhaust registers
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = mod1 + 4;
    int v5 = mod2 + 5;
    int v6 = stride * 2;
    int v7 = offset * 3;
    int v8 = scale * 4;
    int v9 = mod1 * 5;
    int v10 = mod2 * 6;
    int v11 = stride + offset;
    int v12 = scale + mod1;
    int v13 = mod2 + stride;
    int v14 = offset + scale;
    int v15 = mod1 + mod2;
    int v16 = stride * scale;
    int v17 = offset * mod1;
    int v18 = scale * mod2;
    int v19 = stride + mod1 + 1;
    int v20 = offset + mod2 + 2;
    
    int sum = 0;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Complex index calculation - forces address computation
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int idx = (i * v1 + v2) / v3;
        idx = (idx + v4) % v5;
        
        // Complex load with struct field access
        // Mixes array index and struct field - complex addressing mode
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val = val * v6 + v7;
        
        // Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        arr[i].x[0] = val * v8 + v9;
        
        // More complex addressing in load
        int idx2 = (i * v10 + v11) / v12;
        idx2 = (idx2 + v13) % v14;
        
        // Another complex load
        int val2 = arr[idx2].x[(i + 1) % 4] + arr[i].y;
        
        // Complex store with different addressing
        arr[i].x[1] = val2 * v15 + v16;
        
        // Inline assembly to force address reloads
        // Should trigger RELOAD_FOR_OPERAND_ADDRESS
        asm volatile("# Memory operand 1: %0" : : "m" (arr[idx].y));
        
        // Another inline assembly with complex addressing
        // Should trigger RELOAD_FOR_OTHER_ADDRESS
        asm volatile("# Memory operand 2: %0" : : "m" (arr[(i * v17 + v18) / v19].x[2]));
        
        // Use all variables to keep them live
        sum += arr[i].x[0] + arr[i].x[1] + v20;
        
        // Update some variables to prevent optimization
        v1 += i & 1;
        v2 += i & 2;
        v3 += i & 3;
    }
    
    return sum;
}

// Another noinline function to create more reload contexts
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 100;
    }
}

int main() {
    // Volatile to prevent constant propagation
    volatile int N = SIZE;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int mod1 = 17;
    volatile int mod2 = 23;
    
    // Large array to work with
    struct Data arr[SIZE];
    
    // Initialize with pattern
    init_array(arr, N);
    
    // Run complex access loop
    int result = complex_access_loop(arr, N, stride, offset, scale, mod1, mod2);
    
    // Compute checksum to prevent dead code elimination
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].y;
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return 0;
}
