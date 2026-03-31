#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25
#define ARRAY_SIZE 1024

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
volatile int mod1 = 5;
volatile int mod2 = 11;

/* Helper function to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with different values to keep them live */
    v1 = stride;
    v2 = offset;
    v3 = scale;
    v4 = mod1;
    v5 = mod2;
    v6 = n % 13;
    v7 = n % 17;
    v8 = n % 19;
    v9 = n % 23;
    v10 = n % 29;
    v11 = v1 + v2;
    v12 = v3 * v4;
    v13 = v5 - v6;
    v14 = v7 ^ v8;
    v15 = v9 | v10;
    v16 = v11 & v12;
    v17 = v13 + v14;
    v18 = v15 * v16;
    v19 = v17 - v18;
    v20 = v19 % 31;
    v21 = v20 + 1;
    v22 = v21 * 3;
    v23 = v22 / 2;
    v24 = v23 << 1;
    v25 = v24 >> 1;
    
    int sum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many variables */
        int idx = ((i * v1 + v2) / v3 + v4 * (i % v5) - v6 + v7) % (n - 1);
        if (idx < 0) idx = -idx;
        
        /* Complex load with struct field access and array indexing */
        int val1 = arr[idx].x[i % 4] + arr[i - 1].y;
        int val2 = arr[(idx + v8) % n].z[i % 2] * v9;
        
        /* More complex calculations keeping variables live */
        int temp1 = val1 * v10 + v11 - v12 * v13;
        int temp2 = val2 / (v14 + 1) + v15 * (v16 % v17);
        
        /* Complex store with different addressing */
        int store_idx = ((i * v18 + v19) / (v20 + 1) + v21) % n;
        arr[store_idx].x[0] = temp1 + temp2 + v22;
        
        /* Another store with complex addressing */
        int store_idx2 = (i * v23 + v24) % n;
        arr[store_idx2].y = temp1 - temp2 + v25;
        
        /* Inline assembly to force address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[store_idx].x[2]));
        
        /* More inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address: %0" : : "m" (arr[(idx + i) % n].z[0]));
        
        /* Keep all variables live by using them */
        v1 = (v1 + 1) % 7;
        v2 = (v2 + v3) % 11;
        v3 = (v3 + v4) % 13;
        v4 = (v4 + v5) % 17;
        v5 = (v5 + v6) % 19;
        v6 = (v6 + v7) % 23;
        v7 = (v7 + v8) % 29;
        v8 = (v8 + v9) % 31;
        v9 = (v9 + v10) % 37;
        v10 = (v10 + v11) % 41;
        v11 = (v11 + v12) % 43;
        v12 = (v12 + v13) % 47;
        v13 = (v13 + v14) % 53;
        v14 = (v14 + v15) % 59;
        v15 = (v15 + v16) % 61;
        v16 = (v16 + v17) % 67;
        v17 = (v17 + v18) % 71;
        v18 = (v18 + v19) % 73;
        v19 = (v19 + v20) % 79;
        v20 = (v20 + v21) % 83;
        v21 = (v21 + v22) % 89;
        v22 = (v22 + v23) % 97;
        v23 = (v23 + v24) % 101;
        v24 = (v24 + v25) % 103;
        v25 = (v25 + i) % 107;
        
        sum += arr[store_idx].x[0] + arr[store_idx2].y;
    }
    
    return sum;
}

/* Another function to trigger output address reloads */
__attribute__((noinline, noipa))
void complex_store_operations(struct Data *arr, int n, int *output) {
    volatile int s1 = 2, s2 = 3, s3 = 5, s4 = 7, s5 = 11;
    
    for (int i = 0; i < n; ++i) {
        /* Complex output address calculation */
        int out_idx = (i * s1 + s2 * s3 - s4 * (i % s5)) % n;
        
        /* Complex store to output array - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        output[out_idx] = arr[i].x[i % 4] * arr[(i + 1) % n].y;
        
        /* More complex addressing for another store */
        int out_idx2 = ((i + s1) * s2 + s3) % n;
        output[out_idx2] += arr[i].z[i % 2] + s4;
        
        /* Inline assembly with output addresses */
        asm volatile("# Output address: %0" : : "m" (output[out_idx]));
    }
}

int main() {
    /* Non-constant loop bound */
    volatile int N = ARRAY_SIZE;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    int *output = (int*)malloc(N * sizeof(int));
    
    if (!arr || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z[0] = (i * 29 + 3) % 100;
        arr[i].z[1] = (i * 31 + 11) % 100;
        output[i] = 0;
    }
    
    /* Call functions that trigger complex reloads */
    int sum1 = complex_access_loop(arr, N);
    complex_store_operations(arr, N, output);
    
    /* Compute checksum to prevent dead code elimination */
    int total_sum = sum1;
    for (int i = 0; i < N; ++i) {
        total_sum += output[i];
        for (int j = 0; j < 4; ++j) {
            total_sum += arr[i].x[j];
        }
        total_sum += arr[i].y + arr[i].z[0] + arr[i].z[1];
    }
    
    printf("Checksum: %d\n", total_sum);
    
    free(arr);
    free(output);
    
    return 0;
}
