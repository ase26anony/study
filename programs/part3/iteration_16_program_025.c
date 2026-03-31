#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 25

// Complex struct to force multi-level addressing
struct Data {
    int x[4];
    int y;
    int z[2];
};

// Prevent optimization of helper functions
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n, 
                         volatile int stride, volatile int offset, 
                         volatile int scale, volatile int mod) {
    // Declare many local variables to exhaust registers
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = mod + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = mod * 2;
    int v9 = stride + offset;
    int v10 = scale + mod;
    int v11 = stride - offset;
    int v12 = scale - mod;
    int v13 = stride * offset;
    int v14 = scale * mod;
    int v15 = stride / 2;
    int v16 = offset / 2;
    int v17 = scale / 2;
    int v18 = mod / 2;
    int v19 = stride % 7;
    int v20 = offset % 7;
    int v21 = scale % 7;
    int v22 = mod % 7;
    int v23 = v1 + v2;
    int v24 = v3 + v4;
    int v25 = v5 + v6;
    
    // Complex loop with addressing that requires multiple reloads
    for (int i = 1; i < n; ++i) {
        // Complex index calculation - forces address computation
        // This will likely need RELOAD_FOR_INPUT_ADDRESS
        int idx = (i * v1 + v2) / (v3 ? v3 : 1);
        idx = idx % (n - 1);
        if (idx < 0) idx = -idx;
        
        // Even more complex second index
        // Forces RELOAD_FOR_INPADDR_ADDRESS
        int idx2 = (i * v5 + v6 * v7 - v8) / (v9 ? v9 : 1);
        idx2 = idx2 % (n - 1);
        if (idx2 < 0) idx2 = -idx2;
        
        // Load with complex addressing - input address reloads
        // arr[idx].x[i % 4] requires base + idx*sizeof(struct) + (i%4)*sizeof(int)
        int val1 = arr[idx].x[i % 4] + arr[i - 1].y;
        
        // Another load with different complex addressing
        // Forces RELOAD_FOR_OTHER_ADDRESS
        int val2 = arr[idx2].z[i % 2] * v10;
        
        // Store with complex addressing - output address reloads
        // This should trigger RELOAD_FOR_OUTPUT_ADDRESS
        arr[i].x[0] = val1 * v11 + val2 * v12;
        
        // Another store with different addressing
        // Forces RELOAD_FOR_OUTADDR_ADDRESS
        arr[i].z[0] = (val1 + val2) * v13 / (v14 ? v14 : 1);
        
        // Inline assembly to force operand address reloads
        // RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v15 + v16) % n].y));
        
        // Another inline assembly with different addressing
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[idx2].x[(v17 + i) % 4]));
        
        // Use all variables to keep them live
        v1 += v18;
        v2 += v19;
        v3 += v20;
        v4 += v21;
        v5 += v22;
        v6 += v23;
        v7 += v24;
        v8 += v25;
        v9 += v1;
        v10 += v2;
        v11 += v3;
        v12 += v4;
        v13 += v5;
        v14 += v6;
        v15 += v7;
        v16 += v8;
        v17 += v9;
        v18 += v10;
        v19 += v11;
        v20 += v12;
        v21 += v13;
        v22 += v14;
        v23 += v15;
        v24 += v16;
        v25 += v17;
    }
    
    // Use all variables in final calculation to prevent dead code elimination
    int dummy = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                v21 + v22 + v23 + v24 + v25;
    arr[0].y = dummy;
}

__attribute__((noinline, noipa))
int compute_checksum(struct Data *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i].x[0];
        sum += arr[i].y;
        sum += arr[i].z[0];
        for (int j = 0; j < 4; ++j) {
            sum += arr[i].x[j];
        }
    }
    return sum;
}

int main() {
    // Volatile to prevent constant propagation
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int mod = 5;
    
    // Allocate and initialize array
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    // Initialize with pattern
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 23) % 100;
        }
        arr[i].y = (i * 31) % 100;
        arr[i].z[0] = (i * 47) % 100;
        arr[i].z[1] = (i * 53) % 100;
    }
    
    // Call the complex function
    complex_access_loop(arr, N, stride, offset, scale, mod);
    
    // Compute and print checksum
    int checksum = compute_checksum(arr, N);
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
