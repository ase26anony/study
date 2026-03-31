#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n, volatile int *params) {
    /* Many live variables to exhaust registers */
    int v1 = params[0];
    int v2 = params[1];
    int v3 = params[2] ? params[2] : 1;  /* Avoid division by zero */
    int v4 = params[3];
    int v5 = params[4];
    int v6 = params[5];
    int v7 = params[6];
    int v8 = params[7];
    int v9 = params[8];
    int v10 = params[9];
    int v11 = params[10];
    int v12 = params[11];
    int v13 = params[12];
    int v14 = params[13];
    int v15 = params[14];
    int v16 = params[15];
    int v17 = params[16];
    int v18 = params[17];
    int v19 = params[18];
    int v20 = params[19];
    int v21 = params[20];
    int v22 = params[21];
    int v23 = params[22];
    int v24 = params[23];
    int v25 = params[24];
    
    /* Complex addressing in loads (RELOAD_FOR_INPUT_ADDRESS) */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v3 * v4 - v5) / (v6 ? v6 : 1);
        idx = (idx + v7 - v8 * v9) % (v10 ? v10 : n);
        if (idx < 0) idx = -idx;
        if (idx >= n) idx = n - 1;
        
        /* Even more complex addressing with struct fields */
        int idx2 = (i * v11 + v12 * v13 - v14) / (v15 ? v15 : 1);
        idx2 = (idx2 + v16 * v17 - v18) % (v19 ? v19 : n);
        if (idx2 < 0) idx2 = -idx2;
        if (idx2 >= n) idx2 = n - 1;
        
        /* Load with complex addressing - triggers RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + 
                  arr[idx2].y * v20 +
                  arr[(i-1) % n].x[(i+1) % 4] * v21;
        
        /* More complex calculations keeping variables live */
        val = val * v22 / (v23 ? v23 : 1) + v24 - v25;
        
        /* Store with different complex addressing - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        int store_idx = (i * v3 + v4 * v5 - v6) / (v7 ? v7 : 1);
        store_idx = (store_idx + v8 * v9 - v10) % (v11 ? v11 : n);
        if (store_idx < 0) store_idx = -store_idx;
        if (store_idx >= n) store_idx = n - 1;
        
        arr[store_idx].x[0] = val;
        
        /* Inline assembly with memory constraint - triggers RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile("# Complex address: %0" 
                     : 
                     : "m" (arr[idx].y), "m" (arr[store_idx].x[0]));
        
        /* Another inline assembly with output memory constraint */
        asm volatile("# Output address: %0" 
                     : "=m" (arr[(i * v12) % n].x[1])
                     : 
                     : "memory");
        
        /* Update some variables to keep them live and varying */
        v1 = v1 + v2;
        v2 = v2 - v3;
        v3 = v3 * v4;
        v4 = v4 / (v5 ? v5 : 1);
    }
}

__attribute__((noinline, noipa))
int compute_checksum(struct Data *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3] + arr[i].y;
    }
    return sum;
}

struct Data {
    int x[4];
    int y;
};

int main() {
    volatile int N = 1024;  /* Prevent constant propagation */
    int n = N;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
    }
    
    /* Create volatile parameters to prevent optimization */
    volatile int params[NUM_VARS];
    for (int i = 0; i < NUM_VARS; ++i) {
        params[i] = (i * 19 + 11) % 50 + 1;  /* Non-zero values */
    }
    
    /* Perform complex accesses */
    complex_access_loop(arr, n, (int*)params);
    
    /* Compute and print checksum */
    int checksum = compute_checksum(arr, n);
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
