#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Complex struct to force multi-level addressing */
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
    /* Many live variables to exhaust registers */
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
    int v11 = stride - offset;
    int v12 = scale - mod;
    int v13 = stride * offset;
    int v14 = scale * mod;
    int v15 = stride / 3;
    int v16 = offset / 3;
    int v17 = scale / 3;
    int v18 = mod / 3;
    int v19 = (stride << 2) | 1;
    int v20 = (offset << 1) | 1;
    int v21 = (scale << 3) | 1;
    int v22 = (mod << 2) | 1;
    int v23 = stride ^ offset;
    int v24 = scale ^ mod;
    int v25 = ~stride;
    
    int sum = 0;
    
    /* Loop with complex addressing that will require multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) / v3) % v4;
        idx = (idx + v5 - v6) * v7 / v8;
        idx = (idx ^ v9) & v10;
        
        /* Ensure idx stays in bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        /* 
         * Complex load with multi-level addressing:
         * arr[idx].x[i % 4] - requires computing arr + idx*sizeof(struct) + offsetof(x) + (i%4)*sizeof(int)
         * This should trigger RELOAD_FOR_INPUT_ADDRESS
         */
        int val1 = arr[idx].x[i % 4];
        
        /* Another complex load with different addressing */
        int val2 = arr[i - 1].y;
        
        /* Use many live variables in computation to keep them alive */
        int val3 = val1 + val2 + v11 - v12 + v13 / v14;
        val3 = (val3 * v15) / v16 + v17 - v18;
        val3 = (val3 ^ v19) & v20 | v21;
        
        /* 
         * Complex store with addressing that differs from loads:
         * arr[i].x[0] = ... - requires computing arr + i*sizeof(struct) + offsetof(x)
         * This should trigger RELOAD_FOR_OUTPUT_ADDRESS
         */
        arr[i].x[0] = val3 * v22 + v23 - v24 + v25;
        
        /* 
         * Inline assembly with memory constraint to force address reloads.
         * This should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR
         */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v1 + v2) % n].y));
        
        /* Another asm with different addressing for other reload types */
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[i].z[i % 2]));
        
        /* Use the result to prevent dead code elimination */
        sum += arr[i].x[0];
        
        /* Modify some variables to prevent optimization */
        v1 += i & 1;
        v2 += i & 2;
        v3 += i & 4;
        v4 += i & 8;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
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
    volatile int stride = 7;
    volatile int offset = 11;
    volatile int scale = 3;
    volatile int mod = 13;
    
    /* Allocate array with dynamic size to prevent optimization */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with non-constant pattern */
    init_array(arr, N);
    
    /* Perform complex accesses that should trigger various reload types */
    int result = complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex operation to trigger more reloads */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        /* Complex addressing in both load and store */
        int idx = (i * stride + offset) / scale;
        if (idx >= N) idx = idx % N;
        
        /* Mix of different addressing modes */
        checksum += arr[idx].x[i % 4];
        arr[i].y = checksum % 1000;
        
        /* Another inline asm for good measure */
        asm volatile("# Final check %0" : : "m" (arr[i].x[0]));
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
