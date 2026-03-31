#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Structure with multiple fields to force complex addressing */
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
    /* Declare many local variables to consume registers */
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
    int v11 = stride * offset;
    int v12 = scale * mod;
    int v13 = v1 + v2;
    int v14 = v3 + v4;
    int v15 = v5 + v6;
    int v16 = v7 + v8;
    int v17 = v9 + v10;
    int v18 = v11 + v12;
    int v19 = v13 + v14;
    int v20 = v15 + v16;
    int v21 = v17 + v18;
    int v22 = v19 + v20;
    int v23 = v21 + v22;
    int v24 = v23 * 2;
    int v25 = v24 / 3;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / (v3 ? v3 : 1);
        idx = (idx * v4 + v5) % (v6 ? v6 : 1);
        idx = (idx + v7 * v8) / (v9 ? v9 : 1);
        
        /* Ensure idx stays in bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(idx + v10) % n].z;
        val += arr[(i * v11 + v12) % n].x[(i + v13) % 4];
        
        /* More complex calculations keeping variables live */
        int temp1 = val * v14 + v15;
        int temp2 = temp1 / (v16 ? v16 : 1) + v17;
        int temp3 = temp2 * v18 - v19;
        int temp4 = temp3 % (v20 ? v20 : 1) + v21;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = temp4 + v22;
        arr[i].x[1] = temp4 * v23;
        arr[i].x[2] = temp4 / (v24 ? v24 : 1);
        arr[i].x[3] = temp4 % (v25 ? v25 : 1);
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v1 + v2) % n].y));
        
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[i].x[(i + v3) % 4]));
        
        /* More complex addressing in inline assembly */
        int complex_idx = (i * v4 + v5 * v6) / (v7 ? v7 : 1);
        asm volatile("# Third address %0" 
                     : 
                     : "m" (arr[complex_idx % n].z));
        
        /* Keep all variables live by using them in calculations */
        v1 = (v1 + 1) % 100;
        v2 = (v2 + 2) % 100;
        v3 = (v3 + 3) % 100;
        v4 = (v4 + 4) % 100;
        v5 = (v5 + 5) % 100;
        v6 = (v6 + 6) % 100;
        v7 = (v7 + 7) % 100;
        v8 = (v8 + 8) % 100;
        v9 = (v9 + 9) % 100;
        v10 = (v10 + 10) % 100;
        v11 = (v11 + 11) % 100;
        v12 = (v12 + 12) % 100;
        v13 = (v13 + 13) % 100;
        v14 = (v14 + 14) % 100;
        v15 = (v15 + 15) % 100;
        v16 = (v16 + 16) % 100;
        v17 = (v17 + 17) % 100;
        v18 = (v18 + 18) % 100;
        v19 = (v19 + 19) % 100;
        v20 = (v20 + 20) % 100;
        v21 = (v21 + 21) % 100;
        v22 = (v22 + 22) % 100;
        v23 = (v23 + 23) % 100;
        v24 = (v24 + 24) % 100;
        v25 = (v25 + 25) % 100;
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
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int mod = 11;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 23) % 1000;
        }
        arr[i].y = (i * 31) % 1000;
        arr[i].z = (i * 47) % 1000;
    }
    
    /* Call the complex access function */
    complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Compute and print checksum to prevent dead code elimination */
    int checksum = compute_checksum(arr, N);
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
