#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define ARRAY_SIZE 1024

/* Complex struct to force multi-level addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod_factor = 11;

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Many live variables to exhaust registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod_factor;
    int v5 = n * 2;
    int v6 = n / 2;
    int v7 = v1 + v2;
    int v8 = v3 * v4;
    int v9 = v5 - v6;
    int v10 = v7 ^ v8;
    int v11 = v9 & 0xFF;
    int v12 = v10 | v11;
    int v13 = v12 << 2;
    int v14 = v13 >> 1;
    int v15 = v14 + 17;
    int v16 = v15 * 3;
    int v17 = v16 % 19;
    int v18 = v17 + 23;
    int v19 = v18 ^ 0x55;
    int v20 = v19 & 0x3F;
    
    int checksum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) / v3 + v4) % v5;
        if (idx < 0) idx = -idx;
        if (idx >= n) idx = i % n;
        
        /* Complex load with struct field access and array indexing */
        int val = arr[idx].x[i % 4] + 
                  arr[i-1].y * v6 +
                  arr[(i * v7) % n].z[0] / (v8 ? v8 : 1);
        
        /* More complex calculations keeping variables live */
        val = (val * v9 + v10) ^ (v11 + v12);
        val = (val & v13) | (v14 << (i & 3));
        val = val + v15 - v16 + v17 * v18;
        
        /* Complex store with different addressing */
        int store_idx = ((i * v19 + v20) / (v1 ? v1 : 1) + v2) % n;
        if (store_idx < 0) store_idx = -store_idx;
        
        arr[store_idx].x[0] = val;
        arr[store_idx].x[1] = val * v3;
        arr[store_idx].x[2] = val + v4;
        arr[store_idx].x[3] = val ^ v5;
        arr[store_idx].y = store_idx * v6;
        arr[store_idx].z[0] = i * v7;
        arr[store_idx].z[1] = idx * v8;
        
        /* Inline assembly to force address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[((i * v9 + v10) / (v11 ? v11 : 1)) % n].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[store_idx].z[(i * v12) % 2]));
        
        checksum += val + store_idx + idx;
        
        /* Update some variables to prevent dead code elimination */
        v1 = (v1 + i) & 0xFF;
        v2 = (v2 * 3 + 1) & 0xFF;
        v3 = (v3 ^ i) & 0xFF;
        v4 = (v4 + v5) & 0xFF;
        v5 = (v5 - v6) & 0xFF;
        v6 = (v6 * 5) & 0xFF;
    }
    
    return checksum;
}

/* Another noinline function to force output address reloads */
__attribute__((noinline, noipa))
void complex_store_operation(struct Data *arr, int idx, int value, 
                            int v1, int v2, int v3, int v4) {
    /* Complex output addressing - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
    int out_idx = ((idx * v1 + v2) * v3) / (v4 ? v4 : 1);
    if (out_idx < 0) out_idx = -out_idx;
    if (out_idx >= ARRAY_SIZE) out_idx = idx % ARRAY_SIZE;
    
    arr[out_idx].y = value * v1 + v2 * v3 - v4;
    arr[out_idx].z[0] = (value ^ v1) | (v2 & v3);
    
    /* Inline assembly forcing output address reload */
    /* RELOAD_FOR_OUTADDR_ADDRESS */
    int temp = value + 1;
    asm volatile("# Output address constraint %0" 
                 : "=m" (arr[((out_idx * v2 + v3) / (v1 ? v1 : 1)) % ARRAY_SIZE].x[0])
                 : "r" (temp));
}

int main() {
    /* Non-constant loop bound */
    volatile int N = ARRAY_SIZE;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(sizeof(struct Data) * ARRAY_SIZE);
    if (!arr) return 1;
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z[0] = (i * 31) % 100;
        arr[i].z[1] = (i * 37) % 100;
    }
    
    /* Call the complex access function */
    int result1 = complex_access_loop(arr, N);
    
    /* Additional complex stores to trigger output address reloads */
    for (int i = 0; i < 100; ++i) {
        complex_store_operation(arr, i, result1 + i, 
                               stride + i, offset - i, 
                               scale * 2, mod_factor % 10);
    }
    
    /* Compute final checksum */
    int final_checksum = result1;
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        final_checksum += arr[i].x[0] + arr[i].y + arr[i].z[0];
    }
    
    /* Print result to prevent dead code elimination */
    printf("Checksum: %d\n", final_checksum);
    
    free(arr);
    return 0;
}
