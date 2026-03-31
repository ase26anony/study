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
volatile int mod_factor = 5;

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, int s, int o, int sc, int mf) {
    /* Many live variables to exhaust registers */
    int v1 = s * 2;
    int v2 = o + 1;
    int v3 = sc - 1;
    int v4 = mf * 3;
    int v5 = s + o;
    int v6 = sc * 2;
    int v7 = mf + s;
    int v8 = o * 3;
    int v9 = sc + mf;
    int v10 = s - o;
    int v11 = mf * sc;
    int v12 = s + sc;
    int v13 = o * mf;
    int v14 = sc - mf;
    int v15 = s * mf;
    int v16 = o + sc;
    int v17 = mf - s;
    int v18 = sc * o;
    int v19 = s * sc;
    int v20 = o - mf;
    
    int sum = 0;
    
    /* Loop with complex addressing that requires multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation spills */
        int idx = (i * v1 + v2) / v3;
        idx = idx % (n - 1);
        if (idx < 0) idx = -idx;
        
        /* Complex addressing for LOAD (input address reloads) */
        /* RELOAD_FOR_INPUT_ADDRESS should be triggered here */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* Use all live variables in computation to keep them alive */
        val += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        val += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Complex addressing for STORE (output address reloads) */
        /* RELOAD_FOR_OUTPUT_ADDRESS should be triggered here */
        int store_idx = (i * v4 + v5) / v6;
        store_idx = store_idx % n;
        if (store_idx < 0) store_idx = -store_idx;
        
        /* Multi-level struct access with array indexing */
        arr[store_idx].x[(i + v7) % 4] = val * v8 + v9;
        
        /* Even more complex addressing for another store */
        /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        int idx2 = (i * v10 + v11) / v12;
        idx2 = idx2 % n;
        if (idx2 < 0) idx2 = -idx2;
        
        arr[idx2].z[i % 2] = (val + v13) * v14;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v15 + v16) / v17 % n].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[idx].x[(i * v18) % 4]));
        
        /* Use complex address as part of expression */
        /* RELOAD_FOR_INPADDR_ADDRESS */
        int complex_addr_val = arr[(idx * v19 + v20) % n].y;
        sum += complex_addr_val + arr[store_idx].x[0];
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23) % 100;
        arr[i].z[0] = (i * 29) % 100;
        arr[i].z[1] = (i * 31) % 100;
    }
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = ARRAY_SIZE;
    int actual_n = N;
    
    /* Allocate array dynamically to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(actual_n * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    init_array(arr, actual_n);
    
    /* Execute complex access pattern */
    int result = complex_access_loop(arr, actual_n, 
                                     stride, offset, scale, mod_factor);
    
    /* Additional complex operations to increase pressure */
    for (int i = 0; i < 10; ++i) {
        /* Mix of different addressing modes */
        int idx = (i * stride + offset) / scale;
        idx = idx % actual_n;
        
        /* Force more reload types */
        arr[idx].x[i % 4] += result;
        
        /* Inline assembly with memory constraint */
        asm volatile("# Final touch %0" : : "m" (arr[i % actual_n].y));
    }
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < actual_n; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z[0] + arr[i].z[1];
        checksum = (checksum * 31) % 10007;
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
