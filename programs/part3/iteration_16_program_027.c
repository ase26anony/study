#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Structure with nested arrays to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int mod) {
    /* Declare many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod;
    int v5 = n % 7;
    int v6 = n % 11;
    int v7 = n % 13;
    int v8 = n % 17;
    int v9 = n % 19;
    int v10 = n % 23;
    int v11 = stride + 1;
    int v12 = offset + 2;
    int v13 = scale + 3;
    int v14 = mod + 4;
    int v15 = v1 * v2;
    int v16 = v3 * v4;
    int v17 = v5 + v6;
    int v18 = v7 + v8;
    int v19 = v9 + v10;
    int v20 = v11 + v12;
    int v21 = v13 + v14;
    int v22 = v15 + v16;
    int v23 = v17 + v18;
    int v24 = v19 + v20;
    int v25 = v21 + v22;
    
    int sum = 0;
    
    /* Loop with complex addressing that will require multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) / v3 + v4) % v5;
        idx = (idx + v6 * v7 - v8) & (n - 1);
        
        /* Force RELOAD_FOR_INPUT_ADDRESS: complex addressing for load */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(i * v9 + v10) % n].z[0];
        
        /* More complex calculations keeping variables live */
        int tmp1 = v11 * i + v12;
        int tmp2 = v13 * idx + v14;
        int tmp3 = v15 * val + v16;
        int tmp4 = v17 * tmp1 + v18;
        int tmp5 = v19 * tmp2 + v20;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS: complex addressing for store */
        arr[i].x[0] = val * v21 + tmp1;
        arr[i].x[1] = tmp2 * v22 + tmp3;
        arr[i].x[2] = tmp4 * v23 + tmp5;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS via inline assembly */
        /* Input address reload */
        asm volatile("# Input address: %0" : : "m" (arr[idx].y));
        
        /* Output address reload */
        asm volatile("# Output address: %0" : "=m" (arr[i].z[1]) : : "memory");
        
        /* Operand address reload with complex addressing */
        int complex_idx = (i * v24 + v25) % n;
        asm volatile("# Complex operand: %0" : : "m" (arr[complex_idx].x[(i + v1) % 4]));
        
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
        
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2];
    }
    
    /* Use all variables one more time to ensure they're live */
    sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    sum += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    sum += v21 + v22 + v23 + v24 + v25;
    
    return sum;
}

/* Another noinline function to force more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23) % 100;
        arr[i].z[0] = (i * 29) % 100;
        arr[i].z[1] = (i * 31) % 100;
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 3;
    volatile int offset = 7;
    volatile int scale = 2;
    volatile int mod = 100;
    
    /* Allocate array with dynamic size */
    struct Data *arr = (struct Data *)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array */
    init_array(arr, N);
    
    /* Perform complex accesses that should trigger various reload types */
    int result = complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Use result to prevent dead code elimination */
    printf("Result checksum: %d\n", result);
    
    /* Additional complex access in main to trigger more reload contexts */
    int extra_sum = 0;
    for (int i = 0; i < 100; ++i) {
        /* Force RELOAD_FOR_INPADDR_ADDRESS and RELOAD_FOR_OUTADDR_ADDRESS */
        int idx1 = (i * stride + offset) / scale;
        int idx2 = (i * offset + stride) % scale;
        
        /* Complex addressing in both load and store */
        int val = arr[idx1 % N].x[idx2 % 4];
        arr[(idx1 + idx2) % N].y = val * stride + offset;
        
        /* Inline assembly with memory operand */
        asm volatile("# Mixed address: %0" : "=m" (arr[i % N].z[i % 2]) : : "memory");
        
        extra_sum += arr[i % N].x[0];
    }
    
    printf("Extra sum: %d\n", extra_sum + result);
    
    free(arr);
    return 0;
}
