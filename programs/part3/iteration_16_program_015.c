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
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int mod1 = 5;
volatile int mod2 = 11;

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Many live variables to exhaust registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod1;
    int v5 = mod2;
    int v6 = n * 2;
    int v7 = n / 3;
    int v8 = n + 100;
    int v9 = n - 50;
    int v10 = v1 * v2;
    int v11 = v3 + v4;
    int v12 = v5 - v6;
    int v13 = v7 * v8;
    int v14 = v9 / 2;
    int v15 = v10 + v11;
    int v16 = v12 * v13;
    int v17 = v14 + v15;
    int v18 = v16 - v17;
    int v19 = v18 * 3;
    int v20 = v19 / 7;
    
    int sum = 0;
    
    /* Loop with complex addressing that requires multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation spills */
        int idx = (i * v1 + v2) / v3;
        idx = (idx + v4) % v5;
        idx = (idx * v6 + v7) / v8;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        /* Complex load with struct field access - triggers RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(idx * v9 + v10) % n].z[0];
        val -= arr[(i * v11 + v12) % n].z[1];
        
        /* Use many live variables in computation to keep them alive */
        val += v13 - v14 + v15 * v16 / (v17 + 1);
        
        /* Complex store - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        int store_idx = (i * v18 + v19) / (v20 + 1);
        store_idx = store_idx % n;
        arr[store_idx].x[(i + v1) % 4] = val * v4 + v5;
        
        /* Another store with different complex addressing */
        int store_idx2 = (i * v6 + v7) / (v8 + v9);
        store_idx2 = store_idx2 % n;
        arr[store_idx2].y = val - v10 + v11 * v12;
        
        /* Inline assembly to force address reloads */
        /* Triggers RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v13 + v14) % n].x[(i + v15) % 4])
                     : "memory");
        
        /* Another asm with output address */
        asm volatile("# Output address %0" 
                     : "=m" (arr[(i * v16 + v17) % n].z[i % 2])
                     :
                     : "memory");
        
        /* Mix in more complex addressing */
        sum += arr[(i * v18 + v19) % n].x[0] 
               + arr[(i * v20 + v1) % n].y 
               - arr[(i * v2 + v3) % n].z[0];
        
        /* Update some variables to prevent dead code elimination */
        v1 = (v1 + i) % 13;
        v2 = (v2 + val) % 17;
        v3 = (v3 + store_idx) % 19;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 200;
        arr[i].z[0] = (i * 3) % 150;
        arr[i].z[1] = (i * 5) % 250;
    }
}

int main() {
    /* Use volatile to prevent compile-time computation */
    volatile int N = ARRAY_SIZE;
    
    /* Large array to work with */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    init_array(arr, N);
    
    /* Perform complex accesses that should trigger various reload types */
    int result = complex_access_loop(arr, N);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Additional complex computation using inline assembly */
    int final_check = 0;
    for (int i = 0; i < 10; ++i) {
        /* More inline assembly with complex addressing */
        asm volatile("# Final check %0" 
                     : "=m" (arr[(i * stride + offset) % N].x[i % 4])
                     :
                     : "memory");
        final_check += arr[i].y;
    }
    
    printf("Final check: %d\n", final_check);
    
    free(arr);
    return 0;
}
