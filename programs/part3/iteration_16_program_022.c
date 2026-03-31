#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(volatile int n, volatile int stride, 
                         volatile int offset, volatile int scale) {
    /* Force many live variables to consume registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = n + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = n * 2;
    int v9 = stride + offset;
    int v10 = stride - offset;
    int v11 = scale * stride;
    int v12 = offset / 2;
    int v13 = scale + 5;
    int v14 = stride + 6;
    int v15 = offset + 7;
    int v16 = scale + 8;
    int v17 = n + 9;
    int v18 = stride * 3;
    int v19 = offset * 3;
    int v20 = scale * 3;
    int v21 = n * 3;
    int v22 = stride + offset + scale;
    int v23 = stride - offset + scale;
    int v24 = (stride * offset) / scale;
    int v25 = (offset * scale) / stride;
    
    /* Struct with multiple fields to create complex addressing */
    struct Data {
        int x[4];
        int y;
        int z[2];
    };
    
    /* Large array to force memory accesses */
    struct Data arr[1024];
    
    /* Initialize array with some values */
    for (int i = 0; i < 1024; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = i + j;
        }
        arr[i].y = i * 2;
        arr[i].z[0] = i * 3;
        arr[i].z[1] = i * 4;
    }
    
    /* Main loop with complex addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v3 * v4 - v5) / (v6 + v7);
        if (idx < 0) idx = -idx;
        idx = idx % 1024;
        
        /* Complex index for another access */
        int idx2 = (i * v8 + v9 * v10 - v11) / (v12 + 1);
        if (idx2 < 0) idx2 = -idx2;
        idx2 = idx2 % 1024;
        
        /* Complex index for third access */
        int idx3 = (i * v13 + v14 * v15 - v16) / (v17 + 1);
        if (idx3 < 0) idx3 = -idx3;
        idx3 = idx3 % 1024;
        
        /* LOAD with complex addressing - should trigger INPUT address reloads */
        /* arr[idx].x[i % 4] uses: base + idx*sizeof(Data) + (i%4)*sizeof(int) */
        int val1 = arr[idx].x[i % 4] + arr[i-1].y;
        
        /* More complex addressing mixing struct fields */
        int val2 = arr[idx2].z[i % 2] + arr[i].x[0] * v18;
        
        /* STORE with complex addressing - should trigger OUTPUT address reloads */
        /* arr[i].x[0] = ... uses: base + i*sizeof(Data) */
        arr[i].x[0] = val1 * v19 + val2 * v20 + v21;
        
        /* Even more complex store addressing */
        int store_idx = (i * v22 + v23) / (v24 + 1);
        if (store_idx < 0) store_idx = -store_idx;
        store_idx = store_idx % 1024;
        
        /* Store with complex index calculation */
        arr[store_idx].y = val1 * v25 + i;
        
        /* Inline assembly to force OPERAND address reloads */
        /* Memory constraint with complex addressing */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        
        /* Another inline assembly with different complex addressing */
        asm volatile("# Complex address 2: %0" : : "m" (arr[store_idx].z[i % 2]));
        
        /* Force address as operand in assembly */
        int* addr = &arr[idx3].x[2];
        asm volatile("# Address operand: %0" : : "r" (addr));
        
        /* Mix in more variable uses to keep them live */
        v1 = v1 + i;
        v2 = v2 - i;
        v3 = v3 * (i % 7 + 1);
        v4 = v4 / (i % 5 + 1);
        v5 = v5 ^ i;
        v6 = v6 | i;
        v7 = v7 & ~i;
        v8 = v8 + v9;
        v9 = v9 - v10;
        v10 = v10 * v11;
        v11 = v11 / (v12 + 1);
        v12 = v12 ^ v13;
        v13 = v13 | v14;
        v14 = v14 & v15;
        v15 = v15 + v16;
        v16 = v16 - v17;
        v17 = v17 * v18;
        v18 = v18 / (v19 + 1);
        v19 = v19 ^ v20;
        v20 = v20 | v21;
        v21 = v21 & v22;
        v22 = v22 + v23;
        v23 = v23 - v24;
        v24 = v24 * (v25 + 1);
        v25 = v25 / (i % 3 + 1);
    }
    
    /* Compute checksum to prevent elimination */
    int checksum = 0;
    for (int i = 0; i < n && i < 1024; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z[0] + arr[i].z[1];
        checksum = checksum ^ (i * 0x5a5a5a5a);
    }
    
    /* Use checksum to prevent dead code elimination */
    volatile int result = checksum;
    (void)result;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void complex_store_operation(volatile int* base, volatile int index, 
                            volatile int stride, volatile int offset) {
    /* Complex addressing in store */
    int idx = (index * stride + offset) / 2;
    base[idx] = index * stride + offset;
    
    /* Inline assembly with memory constraint */
    asm volatile("# Store address: %0" : : "m" (base[idx * 2]));
}

int main() {
    /* Volatile variables to prevent constant propagation */
    volatile int N = 512;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Call the complex access function */
    complex_access_loop(N, stride, offset, scale);
    
    /* Additional complex store operations */
    int buffer[2048];
    for (volatile int i = 0; i < 100; i++) {
        complex_store_operation(buffer, i, stride, offset);
    }
    
    /* Compute and print result to prevent elimination */
    int sum = 0;
    for (int i = 0; i < 2048; i++) {
        sum += buffer[i];
    }
    
    printf("Result: %d\n", sum);
    return sum % 256;
}
