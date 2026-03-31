#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 20

/* Structure with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static int complex_access_loop(struct Data *arr, int n, 
                               volatile int stride, 
                               volatile int offset, 
                               volatile int scale) {
    /* Declare many local variables to consume registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize with values to keep them live */
    v1 = stride + 1;
    v2 = offset + 2;
    v3 = scale + 3;
    v4 = n * 2;
    v5 = n / 2;
    v6 = stride * 3;
    v7 = offset * 4;
    v8 = scale * 5;
    v9 = n + stride;
    v10 = n - offset;
    v11 = stride + offset;
    v12 = stride - scale;
    v13 = offset + scale;
    v14 = n * stride;
    v15 = n / scale;
    v16 = stride * offset;
    v17 = offset * scale;
    v18 = scale * stride;
    v19 = n + stride + offset;
    v20 = n - offset + scale;
    
    int sum = 0;
    
    /* Loop with complex addressing that forces reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % n;
        idx = (idx + v6 - v7) * v8 / v9;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(i + v10) % n].z;
        val += arr[(idx * v11 + v12) % n].x[(i + v13) % 4];
        
        /* Use more variables in computation to keep them live */
        val += v14 - v15 + v16 - v17 + v18 - v19 + v20;
        
        /* Complex store - triggers output address reloads */
        arr[i].x[0] = val * v1 + v2;
        arr[i].x[1] = val * v3 + v4;
        arr[i].x[2] = val * v5 + v6;
        arr[i].x[3] = val * v7 + v8;
        arr[i].y = val * v9 + v10;
        arr[i].z = val * v11 + v12;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" 
                     : 
                     : "m" (arr[(idx + v13) % n].x[(i + v14) % 4]));
        
        asm volatile("# Complex address 2: %0" 
                     : 
                     : "m" (arr[(i * v15 + v16) / v17].y));
        
        /* More complex addressing in asm */
        asm volatile("# Complex address 3: %0" 
                     : 
                     : "m" (arr[((i * v18 + v19) / v20) % n].z));
        
        /* Use all variables in sum to keep them live */
        sum += val + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    return sum;
}

__attribute__((noinline, noipa))
static void initialize_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 100;
        arr[i].z = (i * 17) % 100;
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Allocate array on heap to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    initialize_array(arr, N);
    
    /* Perform complex accesses to trigger reloads */
    int result = complex_access_loop(arr, N, stride, offset, scale);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3];
        checksum += arr[i].y + arr[i].z;
    }
    
    /* Mix result with checksum */
    checksum = (checksum + result) % 1000000;
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
