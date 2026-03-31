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
    /* Declare many local variables to consume registers */
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
    int v19 = (stride + offset) / 2;
    int v20 = (scale + extra) / 2;
    
    int sum = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation */
        int idx = (i * v1 + v2) / v3;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* Use many variables in computation to keep them live */
        val = val * v4 + v5 - v6 + v7 * v8 - v9 + v10;
        
        /* Complex store - triggers output address reloads */
        arr[i].x[0] = val * v11 + v12 - v13 * v14 + v15;
        
        /* More complex addressing for another store */
        int idx2 = (i * v16 + v17) / v18;
        if (idx2 < 0) idx2 = 0;
        if (idx2 >= n) idx2 = n - 1;
        
        arr[idx2].x[1] = val + v19 - v20;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v1 + v2) / (v3 + 1)].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[idx].x[(i + v4) % 4]));
        
        /* Use all variables to keep them live */
        sum += arr[i].x[0] + arr[idx2].x[1] + v1 + v2 + v3 + v4 + v5 + 
               v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15 + 
               v16 + v17 + v18 + v19 + v20;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void complex_store_operation(struct Data *arr, int n, 
                            volatile int a, volatile int b, 
                            volatile int c, volatile int d) {
    for (int i = 0; i < n; ++i) {
        /* Complex output addressing - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        int idx = (i * a + b) / (c + 1);
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Even more complex addressing with multiple components */
        arr[(idx * b + c) / (d + 1)].x[2] = 
            arr[(i * c + d) / (a + 1)].x[3] * 
            arr[(idx * d + a) / (b + 1)].y;
        
        /* Inline assembly for output address reloads */
        /* RELOAD_FOR_OUTADDR_ADDRESS */
        asm volatile("# Output address %0" 
                     : "=m" (arr[(i * a + b + c) / (d + 2)].x[3]));
    }
}

int main() {
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 5;
    
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 23) % 100;
        }
        arr[i].y = (i * 31) % 100;
    }
    
    /* Call functions that trigger complex reloads */
    int sum1 = complex_access_loop(arr, N, stride, offset, scale, extra);
    
    complex_store_operation(arr, N, stride, offset, scale, extra);
    
    /* Compute checksum */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3] + arr[i].y;
    }
    checksum += sum1;
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
