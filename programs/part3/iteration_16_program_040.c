#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

// Struct with multiple fields to force complex addressing
struct Data {
    int x[4];
    int y;
    int z;
};

// Volatile variables to prevent constant propagation
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int extra1 = 1;
volatile int extra2 = 2;

// Force no optimization on this function
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    // Declare many local variables to consume registers
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = extra1;
    int v5 = extra2;
    int v6 = n + 1;
    int v7 = n * 2;
    int v8 = n / 2;
    int v9 = n % 3;
    int v10 = v1 + v2;
    int v11 = v3 * v4;
    int v12 = v5 - v6;
    int v13 = v7 + v8;
    int v14 = v9 * v10;
    int v15 = v11 / (v12 + 1);
    int v16 = v13 - v14;
    int v17 = v15 + v16;
    int v18 = v17 * 2;
    int v19 = v18 / 3;
    int v20 = v19 % 5;
    int v21 = v20 + v1;
    int v22 = v21 * v2;
    int v23 = v22 / v3;
    int v24 = v23 + v4;
    int v25 = v24 * v5;
    
    int sum = 0;
    
    // Complex loop with multiple addressing modes
    for (int i = 1; i < n; ++i) {
        // Complex index calculation using many live variables
        // This forces address computation to need multiple registers
        int idx = (i * v1 + v2 + v6 - v7) / (v3 + 1);
        idx = (idx * v8 + v9) % (v10 + 1);
        idx = (idx + v11 - v12) * (v13 + 1) / (v14 + 1);
        
        // Ensure idx stays in bounds
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        // Complex load with struct field access and array indexing
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(i * v15) % n].z;
        val += arr[(idx + v16) % n].x[(i + v17) % 4];
        
        // More complex address calculations using many variables
        int idx2 = (i * v18 + v19) / (v20 + 1);
        idx2 = (idx2 + v21) % (v22 + 1);
        idx2 = idx2 % n;
        
        int idx3 = (i * v23 + v24) / (v25 + 1);
        idx3 = (idx3 * v1 + v2) % (v3 + 1);
        idx3 = idx3 % n;
        
        // Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        arr[i].x[0] = val * v4 + v5 + arr[idx2].y - arr[idx3].z;
        
        // Another store with different complex addressing
        // Mixes array index and struct field
        arr[(i * v6 + v7) % n].x[(i * v8) % 4] = 
            arr[i].x[0] + arr[idx].y * v9;
        
        // Inline assembly to force address reloads
        // Should trigger RELOAD_FOR_OPERAND_ADDRESS
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v10 + v11) % n].y));
        
        // Another inline assembly with different addressing
        // Should trigger RELOAD_FOR_OTHER_ADDRESS
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[i].x[(i * v12) % 4]));
        
        // Complex store to trigger output address reloads
        int idx4 = (i * v13 + v14) / (v15 + 1);
        idx4 = idx4 % n;
        arr[idx4].z = arr[i].x[0] * v16 + v17;
        
        // Keep all variables live by using them
        sum += val + idx + idx2 + idx3 + idx4 + 
               v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25;
        
        // Modify some variables to prevent optimization
        v1 = (v1 + 1) % 7;
        v2 = (v2 + i) % 11;
        v3 = (v3 + 2) % 5;
    }
    
    return sum;
}

// Another noinline function to force more reload contexts
__attribute__((noinline, noipa))
void complex_store_operation(struct Data *arr, int n, int *results) {
    volatile int mod1 = 3;
    volatile int mod2 = 5;
    volatile int mod3 = 7;
    
    for (int i = 0; i < n; ++i) {
        // Complex addressing for store
        int store_idx = (i * mod1 + mod2) / (mod3 + 1);
        store_idx = store_idx % n;
        
        // Complex addressing for source
        int src_idx = (i * mod2 + mod3) / (mod1 + 1);
        src_idx = src_idx % n;
        
        // Store with complex addressing - should trigger output address reloads
        arr[store_idx].y = arr[src_idx].x[i % 4] * mod1 + mod2;
        
        // Inline assembly with memory constraint
        // Should trigger various address reload types
        asm volatile("# Store address %0" 
                     : "=m" (arr[(i * mod3) % n].z));
        
        results[i] = arr[store_idx].y;
    }
}

int main() {
    volatile int N = 1024;
    int n = N;
    
    // Allocate and initialize array
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) return 1;
    
    // Initialize with pattern
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 100;
        arr[i].z = (i * 17) % 100;
    }
    
    // Call function with complex addressing
    int sum1 = complex_access_loop(arr, n);
    
    // Additional operations to trigger more reload types
    int *results = (int*)malloc(n * sizeof(int));
    complex_store_operation(arr, n, results);
    
    // Compute checksum
    int checksum = sum1;
    for (int i = 0; i < n; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z;
        checksum += results[i];
    }
    
    // Use checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    
    free(results);
    free(arr);
    
    return 0;
}
