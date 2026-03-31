#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 20

struct Data {
    int x[4];
    int y;
};

/* Prevent optimization of complex addressing calculations */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int extra) {
    /* Declare many local variables to exhaust registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = extra * 2;
    int v9 = stride + offset;
    int v10 = scale + extra;
    int v11 = stride * offset;
    int v12 = scale * extra;
    int v13 = stride - offset;
    int v14 = scale - extra;
    int v15 = stride / 2;
    int v16 = offset / 2;
    int v17 = scale / 2;
    int v18 = extra / 2;
    int v19 = (stride + offset) * scale;
    int v20 = (scale + extra) * stride;
    
    int sum = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation in steps */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % (n - 1);
        idx = (idx + v6) * v7 / v8;
        
        /* Ensure idx stays in bounds */
        if (idx < 0) idx = -idx;
        if (idx >= n) idx = n - 1;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(i * v9 + v10) % n].x[(i + 1) % 4];
        val -= arr[(i * v11 + v12) % n].y;
        
        /* Use many variables to keep them live */
        val += v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        
        /* Complex store - triggers output address reloads */
        int store_idx = (i * v1 + v3) / v2;
        if (store_idx < 0) store_idx = -store_idx;
        if (store_idx >= n) store_idx = n - 1;
        
        arr[store_idx].x[0] = val * v4 + v5;
        arr[store_idx].x[1] = val * v6 + v7;
        arr[store_idx].x[2] = val * v8 + v9;
        arr[store_idx].x[3] = val * v10 + v11;
        arr[store_idx].y = val * v12 + v13;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Operand address constraint %0" 
                     : 
                     : "m" (arr[(idx * v14 + v15) % n].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address constraint %0" 
                     : 
                     : "m" (arr[(store_idx * v16 + v17) % n].x[0]));
        
        /* Complex addressing in output */
        int out_idx = (i * v18 + v19) / v20;
        if (out_idx < 0) out_idx = -out_idx;
        if (out_idx >= n) out_idx = n - 1;
        
        /* Force output address reload with complex calculation */
        arr[out_idx].y = (arr[idx].x[0] * v1 + arr[store_idx].x[1] * v2) / v3;
        
        sum += val;
    }
    
    /* Use all variables in final calculation to prevent dead code elimination */
    sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 31) % 100;
    }
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 11;
    
    /* Allocate array dynamically to prevent stack optimization */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    init_array(arr, N);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, N, stride, offset, scale, extra);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3] + arr[i].y;
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
