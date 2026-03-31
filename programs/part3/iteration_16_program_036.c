#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Structure with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int mod_val = 4;

/* Helper function to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod_val;
    int v5 = n;
    int v6 = v1 + v2;
    int v7 = v3 * v4;
    int v8 = v5 - v6;
    int v9 = v7 + v8;
    int v10 = v1 * v2;
    int v11 = v3 + v4;
    int v12 = v5 * v6;
    int v13 = v7 - v8;
    int v14 = v9 + v10;
    int v15 = v11 * v12;
    int v16 = v13 - v14;
    int v17 = v15 + v16;
    int v18 = v1 * v3;
    int v19 = v2 * v4;
    int v20 = v5 + v6;
    int v21 = v7 * v8;
    int v22 = v9 - v10;
    int v23 = v11 + v12;
    int v24 = v13 * v14;
    int v25 = v15 - v16;
    
    int sum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % (v6 + 1);
        idx = (idx + v7) * v8 / (v9 + 1);
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex addressing for LOAD (input address reloads) */
        /* Mixing array indices, struct fields, and multiple calculations */
        int val = arr[idx].x[i % 4] + 
                  arr[(i * v10 + v11) % n].y + 
                  arr[(idx * v12 + v13) % n].z[i % 2];
        
        /* More complex calculations using many live variables */
        val = val * v14 + v15 - v16 * v17 + v18 / (v19 + 1);
        
        /* Complex addressing for STORE (output address reloads) */
        /* Different complex address for store */
        int store_idx = (i * v20 + v21) / (v22 + 1);
        store_idx = (store_idx * v23 + v24) % (v25 + 1);
        if (store_idx < 0) store_idx = -store_idx;
        store_idx = store_idx % n;
        
        /* Store with complex addressing */
        arr[store_idx].x[(i * v1) % 4] = val + 
                                         arr[(i * v2) % n].y * v3 - 
                                         v4 * arr[(store_idx * v5) % n].z[0];
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v6 + v7) % n].y));
        
        /* Another inline assembly with different addressing */
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[(store_idx * v8 + v9) % n].z[1]));
        
        /* Keep all variables live by using them */
        v1 = (v1 + 1) % 10;
        v2 = (v2 + v3) % 20;
        v3 = (v3 + v4) % 5;
        v4 = (v4 + 1) % 8;
        v5 = (v5 + i) % 100;
        v6 = v6 + v7 - v8;
        v7 = v7 * v9 / (v10 + 1);
        v8 = v8 + v11 - v12;
        v9 = v9 * v13 % (v14 + 1);
        v10 = v10 + v15 - v16;
        
        /* Update sum for checksum */
        sum += val + store_idx + idx;
    }
    
    /* Use all variables in final calculation to keep them live */
    int final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                v21 + v22 + v23 + v24 + v25;
    
    return sum + final;
}

/* Another function to trigger different reload contexts */
__attribute__((noinline, noipa))
void mixed_address_operations(struct Data *arr, int n, int *output) {
    volatile int a = 3, b = 5, c = 7, d = 11;
    
    /* Force RELOAD_FOR_OTHER_ADDRESS */
    for (int i = 0; i < n; i++) {
        /* Complex addressing that doesn't fit simple patterns */
        int idx1 = (i * a + b) % n;
        int idx2 = (i * c + d) % n;
        int idx3 = (idx1 * idx2 + a) % n;
        
        /* Chain of complex addresses */
        output[i] = arr[idx1].x[0] + 
                    arr[idx2].y * arr[idx3].z[0] +
                    arr[(idx1 * b + c) % n].x[1] -
                    arr[(idx2 * d + a) % n].x[2];
        
        /* Inline assembly with memory constraint */
        /* Forces address computation for the operand */
        asm volatile("" 
                     : "=m" (arr[(i * a + b * c) % n].x[3])
                     : 
                     : "memory");
    }
}

int main() {
    const int N = 1024;
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    int *output = (int*)malloc(N * sizeof(int));
    
    if (!arr || !output) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z[0] = (i * 29 + 11) % 100;
        arr[i].z[1] = (i * 31 + 13) % 100;
    }
    
    /* Call functions that trigger complex reloads */
    int result1 = complex_access_loop(arr, N);
    mixed_address_operations(arr, N, output);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = result1;
    for (int i = 0; i < N; i++) {
        checksum += output[i];
        checksum += arr[i].x[0] + arr[i].y;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    free(output);
    
    return 0;
}
