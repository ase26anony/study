#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024

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
volatile int mod1 = 5;
volatile int mod2 = 11;

/* Helper functions marked to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod1;
    int v5 = mod2;
    int v6 = n * 2;
    int v7 = n / 3;
    int v8 = v1 + v2;
    int v9 = v3 * v4;
    int v10 = v5 + v6;
    int v11 = v7 - v8;
    int v12 = v9 ^ v10;
    int v13 = v11 | v12;
    int v14 = v13 & v6;
    int v15 = v14 + v7;
    int v16 = v15 * v8;
    int v17 = v16 / v9;
    int v18 = v17 - v10;
    int v19 = v18 ^ v11;
    int v20 = v19 | v12;
    
    int sum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many variables */
        int idx = ((i * v1 + v2) / v3 + v4) % v5;
        
        /* Ensure idx stays in bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        /* Complex load with struct field access and array indexing */
        int val1 = arr[idx].x[i % 4];
        int val2 = arr[i - 1].y;
        int val3 = arr[(idx + v6) % n].z[i % 2];
        
        /* More complex calculations keeping variables live */
        int tmp1 = val1 * v6 + v7;
        int tmp2 = val2 * v8 - v9;
        int tmp3 = val3 * v10 + v11;
        
        /* Complex store with different addressing */
        int store_idx = ((i * v12 + v13) / v14 + v15) % v16;
        if (store_idx < 0) store_idx = -store_idx;
        store_idx = store_idx % n;
        
        /* Store with complex addressing - triggers output address reloads */
        arr[store_idx].x[0] = tmp1 + tmp2 - tmp3;
        
        /* Another complex store */
        int store_idx2 = ((i * v17 + v18) ^ v19) % v20;
        if (store_idx2 < 0) store_idx2 = -store_idx2;
        store_idx2 = store_idx2 % n;
        
        arr[store_idx2].y = tmp1 * tmp2 + tmp3;
        
        /* Inline assembly to force address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Operand address: %0" : : "m" (arr[idx].y));
        asm volatile("# Other address: %0" : : "m" (arr[store_idx].x[2]));
        
        /* More complex addressing in inline assembly */
        asm volatile("# Complex: %0" : : "m" (arr[(idx + store_idx) % n].z[0]));
        
        /* Update sum to prevent dead code elimination */
        sum += arr[store_idx].x[0] + arr[store_idx2].y;
        
        /* Rotate variables to keep them all live */
        int rot = v1;
        v1 = v2; v2 = v3; v3 = v4; v4 = v5; v5 = v6;
        v6 = v7; v7 = v8; v8 = v9; v9 = v10; v10 = v11;
        v11 = v12; v12 = v13; v13 = v14; v14 = v15; v15 = v16;
        v16 = v17; v17 = v18; v18 = v19; v19 = v20; v20 = rot;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 97;
        }
        arr[i].y = (i * 23 + 7) % 101;
        arr[i].z[0] = (i * 29 + 11) % 103;
        arr[i].z[1] = (i * 31 + 17) % 107;
    }
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = SIZE;
    int actual_n = N;
    
    /* Allocate array with volatile pointer to prevent optimizations */
    struct Data *arr = (struct Data*)malloc(actual_n * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize array */
    init_array(arr, actual_n);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, actual_n);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < actual_n; ++i) {
        checksum += arr[i].x[0] + arr[i].y;
        checksum = (checksum * 31 + 17) % 1000000007;
    }
    
    /* Use result and checksum to prevent elimination */
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
