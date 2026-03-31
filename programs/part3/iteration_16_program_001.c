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
    int v14 = n + stride;
    int v15 = offset + scale;
    int v16 = stride * 3;
    int v17 = offset * 3;
    int v18 = scale * 3;
    int v19 = n * 3;
    int v20 = stride + scale;
    int v21 = offset - stride;
    int v22 = scale * 4;
    int v23 = n + offset;
    int v24 = stride * offset;
    int v25 = scale + offset;
    
    /* Struct with multiple fields to force complex addressing */
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
    
    /* Main loop with complex addressing that should trigger reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % 1024;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        if (idx >= 1024) idx = idx % 1024;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val1 = arr[idx].x[i % 4] + arr[i-1].y;
        int val2 = arr[(i * v6 + v7) / v8].z[0] + arr[idx].z[1];
        
        /* More complex calculations keeping variables live */
        val1 = val1 * v9 + v10;
        val2 = val2 * v11 - v12;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val1 * v13 + v14;
        arr[i].x[1] = val2 * v15 - v16;
        
        /* Even more complex addressing for another store */
        /* Mixing multiple calculations in address */
        int idx2 = (i * v17 + v18) / v19;
        idx2 = (idx2 * v20 + v21) % 1024;
        if (idx2 < 0) idx2 = -idx2;
        if (idx2 >= 1024) idx2 = idx2 % 1024;
        
        arr[idx2].y = val1 + val2 + v22;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[idx2].z[i % 2]));
        
        /* More complex addressing with pointer arithmetic */
        struct Data* ptr = &arr[(i * v23 + v24) / v25 % 1024];
        ptr->x[2] = ptr->x[3] * v1 + v2;
        
        /* Keep all variables live by using them in calculations */
        v1 = v1 + 1; v2 = v2 - 1; v3 = v3 + 2;
        v4 = v4 - 2; v5 = v5 * 2; v6 = v6 / 2;
        v7 = v7 + v8; v8 = v8 - v9; v9 = v9 * v10;
        v10 = v10 / (v11 ? v11 : 1); v11 = v11 + v12;
        v12 = v12 - v13; v13 = v13 * v14; v14 = v14 / (v15 ? v15 : 1);
        v15 = v15 + v16; v16 = v16 - v17; v17 = v17 * v18;
        v18 = v18 / (v19 ? v19 : 1); v19 = v19 + v20;
        v20 = v20 - v21; v21 = v21 * v22; v22 = v22 / (v23 ? v23 : 1);
        v23 = v23 + v24; v24 = v24 - v25; v25 = v25 + 1;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n && i < 1024; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].y;
        for (int j = 0; j < 2; j++) {
            checksum += arr[i].z[j];
        }
    }
    
    /* Use checksum to prevent optimization */
    asm volatile("# Checksum: %0" : : "r" (checksum));
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void trigger_output_address_reloads(volatile int* arr, int size) {
    for (int i = 0; i < size - 1; i++) {
        /* Complex output addressing */
        int idx = (i * 7 + 3) / 2;
        if (idx >= size) idx = idx % size;
        
        /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        arr[idx] = arr[i] * arr[i + 1] + i;
        
        /* Inline assembly with memory constraint */
        asm volatile("# Output address: %0" : : "m" (arr[idx]));
    }
}

int main() {
    /* Volatile to prevent constant propagation */
    volatile int n = 500;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Call function with complex addressing */
    complex_access_loop(n, stride, offset, scale);
    
    /* Create another array for output address reloads */
    int arr2[1000];
    for (int i = 0; i < 1000; i++) {
        arr2[i] = i * 3;
    }
    
    trigger_output_address_reloads(arr2, 1000);
    
    /* Compute final checksum */
    int final_checksum = 0;
    for (int i = 0; i < 1000; i++) {
        final_checksum += arr2[i];
    }
    
    printf("Final checksum: %d\n", final_checksum);
    
    return 0;
}
