#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 20

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static int complex_access_loop(struct Data *arr, int n, 
                               volatile int stride, volatile int offset, 
                               volatile int scale, volatile int extra);

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
static int complex_access_loop(struct Data *arr, int n,
                               volatile int stride, volatile int offset,
                               volatile int scale, volatile int extra) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize with values that depend on parameters */
    v1 = stride + 1;
    v2 = offset + 2;
    v3 = scale + 3;
    v4 = extra + 4;
    v5 = stride * 2;
    v6 = offset * 3;
    v7 = scale * 4;
    v8 = extra * 5;
    v9 = stride + offset;
    v10 = scale + extra;
    v11 = stride * scale;
    v12 = offset * extra;
    v13 = v1 + v2;
    v14 = v3 + v4;
    v15 = v5 + v6;
    v16 = v7 + v8;
    v17 = v9 + v10;
    v18 = v11 + v12;
    v19 = v13 + v14;
    v20 = v15 + v16;
    
    int checksum = 0;
    
    /* Complex loop with addressing that requires multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % (n - 1);
        idx = (idx + v6 * v7) / (v8 + 1);
        
        /* Prevent hoisting of address calculations */
        volatile int temp_idx = idx;
        idx = temp_idx + v9 - v10;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(i * v11 + v12) % n].z;
        
        /* Use more variables in computation to keep them live */
        val = val * v13 / (v14 + 1) + v15;
        val ^= v16;
        val += v17 - v18;
        
        /* Complex store - triggers output address reloads */
        int store_idx = (i * v19 + v20) % n;
        arr[store_idx].x[0] = val;
        
        /* Different complex store to another field */
        int store_idx2 = (store_idx * v1 + v2) % n;
        arr[store_idx2].y = val + v3;
        
        /* Inline assembly to force operand address reloads */
        asm volatile("# Complex address for input: %0" 
                     : 
                     : "m" (arr[(idx * v4 + v5) % n].x[0]));
        
        /* Another inline assembly for different address type */
        asm volatile("# Output address constraint: %0" 
                     : "=m" (arr[(i * v6 + v7) % n].z));
        
        /* Use inline assembly with memory operand that needs address computation */
        asm volatile("" 
                     : 
                     : "m" (arr[store_idx].x[(i + v8) % 4]));
        
        /* Update checksum using complex addressing */
        checksum += arr[(checksum + i) % n].x[0];
        checksum ^= arr[i].y;
        
        /* Modify some variables to prevent dead code elimination */
        v1 += i & 1;
        v2 += i & 2;
        v3 += i & 4;
        v4 += i & 8;
    }
    
    /* Final complex computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    checksum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    
    return checksum;
}

/* Another helper to create more reload contexts */
__attribute__((noinline, noipa))
static void mixed_operations(struct Data *arr, int n, volatile int param) {
    for (int i = 0; i < n - 1; i++) {
        /* Complex addressing for both source and destination */
        int src_idx = (i * param + i * i) % n;
        int dst_idx = (src_idx * 3 + 7) % n;
        
        /* Operation requiring address reloads on both sides */
        arr[dst_idx].x[2] = arr[src_idx].x[1] + arr[i].y;
        
        /* Nested addressing */
        arr[(dst_idx + arr[src_idx].x[0]) % n].z = 
            arr[(src_idx + arr[dst_idx].y) % n].x[3];
    }
}

int main(void) {
    volatile int N = 1024;  /* Non-constant to prevent optimization */
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 11;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data *)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 23) % 1000;
        }
        arr[i].y = (i * 31) % 1000;
        arr[i].z = (i * 47) % 1000;
    }
    
    /* Call functions that trigger complex reloads */
    int result1 = complex_access_loop(arr, N, stride, offset, scale, extra);
    mixed_operations(arr, N, stride);
    
    /* Compute final checksum with more complex addressing */
    int final_checksum = 0;
    for (int i = 0; i < N; i++) {
        int idx = (i * stride + offset) % N;
        final_checksum += arr[idx].x[i % 4];
        final_checksum += arr[(idx + scale) % N].y;
        final_checksum ^= arr[(i * extra) % N].z;
    }
    
    final_checksum += result1;
    
    printf("Checksum: %d\n", final_checksum);
    
    free(arr);
    return 0;
}
