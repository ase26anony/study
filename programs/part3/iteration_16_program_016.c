#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024

struct Data {
    int x[4];
    int y;
    int z;
};

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n, 
                         volatile int stride, volatile int offset, 
                         volatile int scale, volatile int mod) {
    /* Create many live variables to exhaust registers */
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
    int v15 = stride / 3;
    int v16 = offset / 3;
    int v17 = scale / 3;
    int v18 = mod / 3;
    int v19 = (stride << 2) | 1;
    int v20 = (offset << 2) | 1;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation */
        int idx = (i * v1 + v2) / (v3 ? v3 : 1);
        idx = (idx * v4 + v5) % (v6 ? v6 : 1);
        
        /* Even more complex nested index */
        int idx2 = ((i * v7) / (v8 ? v8 : 1)) + v9;
        idx2 = (idx2 * v10) % (v11 ? v11 : 1);
        
        /* Complex addressing for LOAD (input address reloads) */
        /* Mix struct fields and array indices */
        int val1 = arr[idx].x[i % 4] + arr[i-1].y;
        int val2 = arr[idx2].z * arr[(i * v12) % n].x[2];
        
        /* Use many live variables in computation to keep them alive */
        val1 = val1 * v13 + v14 - v15 + v16;
        val2 = val2 / (v17 ? v17 : 1) + v18 * v19 - v20;
        
        /* Complex addressing for STORE (output address reloads) */
        /* Different complex address for store */
        int store_idx = (i * v1 + v3) / (v2 ? v2 : 1);
        store_idx = (store_idx * v5) % (v6 ? v6 : 1);
        
        arr[store_idx].x[0] = val1 + val2;
        arr[store_idx].x[1] = val1 - val2;
        arr[store_idx].x[2] = val1 * val2;
        arr[store_idx].x[3] = val1 / (val2 ? val2 : 1);
        
        /* More complex store with different addressing */
        int store_idx2 = (i * v7 + v9) / (v10 ? v10 : 1);
        arr[store_idx2].y = (val1 << 2) | (val2 & 0xFF);
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Operand address constraint %0" 
                     : 
                     : "m" (arr[idx].y), "m" (arr[store_idx].z));
        
        /* Another inline asm with different addressing */
        asm volatile("# Other address constraint %0 %1" 
                     : 
                     : "m" (arr[(i * v11) % n].x[1]), 
                       "m" (arr[(i * v12) % n].x[3]));
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with more complex asm */
        int complex_idx = (idx * v13 + idx2 * v14) / (v15 ? v15 : 1);
        asm volatile("# Complex address %0" 
                     : 
                     : "m" (arr[complex_idx % n].x[0]));
        
        /* Mix in pointer arithmetic that might need reloads */
        struct Data *ptr1 = &arr[idx];
        struct Data *ptr2 = &arr[store_idx];
        ptr1->z = ptr2->y + i;
        
        /* Use all variables to keep them live */
        v1 = (v1 + i) & 0xFFF;
        v2 = (v2 + idx) & 0xFFF;
        v3 = (v3 + store_idx) & 0xFFF;
        v4 = (v4 + val1) & 0xFFF;
        v5 = (v5 + val2) & 0xFFF;
        v6 = (v6 + complex_idx) & 0xFFF;
        v7 = (v7 * 3 + 1) & 0xFFF;
        v8 = (v8 * 5 + 1) & 0xFFF;
        v9 = (v9 * 7 + 1) & 0xFFF;
        v10 = (v10 * 11 + 1) & 0xFFF;
        v11 = (v11 * 13 + 1) & 0xFFF;
        v12 = (v12 * 17 + 1) & 0xFFF;
        v13 = (v13 * 19 + 1) & 0xFFF;
        v14 = (v14 * 23 + 1) & 0xFFF;
        v15 = (v15 * 29 + 1) & 0xFFF;
        v16 = (v16 * 31 + 1) & 0xFFF;
        v17 = (v17 * 37 + 1) & 0xFFF;
        v18 = (v18 * 41 + 1) & 0xFFF;
        v19 = (v19 * 43 + 1) & 0xFFF;
        v20 = (v20 * 47 + 1) & 0xFFF;
    }
}

__attribute__((noinline, noipa))
int compute_checksum(struct Data *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3];
        sum += arr[i].y + arr[i].z;
    }
    return sum;
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int N = SIZE;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int mod = 11;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 1000;
        }
        arr[i].y = (i * 23 + 7) % 1000;
        arr[i].z = (i * 29 + 11) % 1000;
    }
    
    /* Call the complex function */
    complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Compute and print checksum to prevent elimination */
    int checksum = compute_checksum(arr, N);
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
