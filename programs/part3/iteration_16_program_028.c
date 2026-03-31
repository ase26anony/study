#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MANY_VARS 30

/* Prevent optimization of addressing calculations */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int extra1 = 5, extra2 = 11;

struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Force complex addressing to remain */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many variables to consume registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra1 + 4;
    int v5 = extra2 + 5;
    int v6 = v1 * 2;
    int v7 = v2 * 2;
    int v8 = v3 * 2;
    int v9 = v4 * 2;
    int v10 = v5 * 2;
    int v11 = v1 + v2;
    int v12 = v3 + v4;
    int v13 = v5 + v6;
    int v14 = v7 + v8;
    int v15 = v9 + v10;
    int v16 = v11 * 3;
    int v17 = v12 * 3;
    int v18 = v13 * 3;
    int v19 = v14 * 3;
    int v20 = v15 * 3;
    int v21 = v16 + 1;
    int v22 = v17 + 2;
    int v23 = v18 + 3;
    int v24 = v19 + 4;
    int v25 = v20 + 5;
    int v26 = v21 * v1;
    int v27 = v22 * v2;
    int v28 = v23 * v3;
    int v29 = v24 * v4;
    int v30 = v25 * v5;
    
    int sum = 0;
    
    /* Complex loop with addressing that requires multiple reloads */
    for (int i = 2; i < n - 2; ++i) {
        /* Complex index calculation - forces address computation with many live vars */
        int idx1 = (i * v1 + v2 + v6 - v7) / v3;
        int idx2 = (i * v4 + v5 + v8 - v9) / v10;
        int idx3 = (i * v11 + v12) * v13 / v14;
        
        /* Ensure all variables are live by using them in calculations */
        idx1 = (idx1 + v15 + v16 - v17) & 0xFFF;
        idx2 = (idx2 + v18 + v19 - v20) & 0xFFF;
        idx3 = (idx3 + v21 + v22 - v23) & 0xFFF;
        
        /* Complex load addressing - should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val1 = arr[idx1].x[i % 4] + arr[i-1].y;
        int val2 = arr[idx2].z[i % 2] + arr[i-2].x[3];
        
        /* More complex addressing with struct fields */
        int val3 = arr[idx3].x[(i + v24) % 4] + arr[idx1].y * v25;
        
        /* Complex store addressing - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val1 * v26 + v27;
        arr[i].x[1] = val2 * v28 + v29;
        arr[i].x[2] = val3 * v30 + v1;
        
        /* Even more complex addressing for next store */
        int store_idx = (i * v2 + v3 * v4) / v5;
        store_idx = (store_idx + v6 + v7 - v8) & 0xFFF;
        
        /* Store with complex addressing - different type of output address */
        arr[store_idx].y = val1 + val2 + val3 + v9;
        
        /* Use inline assembly to force specific address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Input address reload: %0" : : "m" (arr[idx1].x[0]));
        asm volatile("# Output address reload: %0" : : "m" (arr[store_idx].y));
        asm volatile("# Complex operand: %0" : : "m" (arr[(i * v10 + v11) / v12].z[0]));
        
        /* Keep all variables live */
        v1 += (i & 1);
        v2 += (i & 2) >> 1;
        v3 += (i & 4) >> 2;
        v4 += (i & 8) >> 3;
        v5 = v5 * 3 / 2;
        
        /* Use remaining variables to ensure they're not optimized out */
        v6 = v6 + v7 - v8 + v9;
        v7 = v7 + v8 - v9 + v10;
        v8 = v8 + v9 - v10 + v11;
        v9 = v9 + v10 - v11 + v12;
        v10 = v10 + v11 - v12 + v13;
        v11 = v11 + v12 - v13 + v14;
        v12 = v12 + v13 - v14 + v15;
        v13 = v13 + v14 - v15 + v16;
        v14 = v14 + v15 - v16 + v17;
        v15 = v15 + v16 - v17 + v18;
        v16 = v16 + v17 - v18 + v19;
        v17 = v17 + v18 - v19 + v20;
        v18 = v18 + v19 - v20 + v21;
        v19 = v19 + v20 - v21 + v22;
        v20 = v20 + v21 - v22 + v23;
        v21 = v21 + v22 - v23 + v24;
        v22 = v22 + v23 - v24 + v25;
        v23 = v23 + v24 - v25 + v26;
        v24 = v24 + v25 - v26 + v27;
        v25 = v25 + v26 - v27 + v28;
        v26 = v26 + v27 - v28 + v29;
        v27 = v27 + v28 - v29 + v30;
        v28 = v28 + v29 - v30 + v1;
        v29 = v29 + v30 - v1 + v2;
        v30 = v30 + v1 - v2 + v3;
        
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[store_idx].y;
    }
    
    /* Use all variables in final calculation to prevent optimization */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
}

/* Another function with different addressing patterns */
__attribute__((noinline, noipa))
void complex_store_operations(struct Data *arr, int n, int *output) {
    volatile int mod1 = 17, mod2 = 23, mod3 = 29;
    
    for (int i = 1; i < n; ++i) {
        /* Complex output addressing - should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        int out_idx = (i * mod1 + i * mod2) / mod3;
        out_idx = (out_idx * 7 + 3) & 0xFFF;
        
        /* Complex input for output address calculation */
        int in_idx = (i * 11 + 5) & 0xFFF;
        
        /* Store with address that itself needs reloads */
        arr[out_idx].z[i % 2] = arr[in_idx].x[(i + 1) % 4] * 3 + 7;
        
        /* Inline assembly with memory operand - different reload context */
        asm volatile("# Operand address: %0" : : "m" (arr[out_idx].z[0]));
        
        /* Chain of dependent addresses */
        int idx2 = (out_idx * 3 + 1) & 0xFFF;
        output[i] = arr[idx2].y + arr[out_idx].z[0];
    }
}

int main() {
    volatile int N = 1024;
    int actual_n = N;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)calloc(actual_n, sizeof(struct Data));
    int *output = (int*)malloc(actual_n * sizeof(int));
    
    if (!arr || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with some data */
    for (int i = 0; i < actual_n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 1000;
        }
        arr[i].y = (i * 23 + 7) % 1000;
        arr[i].z[0] = (i * 29 + 11) % 1000;
        arr[i].z[1] = (i * 31 + 17) % 1000;
    }
    
    /* Call functions that trigger complex reloads */
    int result1 = complex_access_loop(arr, actual_n);
    complex_store_operations(arr, actual_n, output);
    
    /* Compute checksum to prevent optimization */
    int checksum = result1;
    for (int i = 0; i < actual_n && i < 100; ++i) {
        checksum += arr[i].x[0] + arr[i].y + output[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    free(output);
    
    return 0;
}
