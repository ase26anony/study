#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
#define NOOPT __attribute__((noinline, noipa))

/* Complex struct to force multi-level addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Force runtime values, prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int extra1 = 5;
volatile int extra2 = 11;

/* Helper to force complex addressing in loads */
NOOPT int complex_load(struct Data *arr, int i, int v1, int v2, int v3, int v4) {
    /* Complex index calculation with many live variables */
    int idx = (i * v1 + v2) / v3;
    
    /* Prevent hoisting of address calculation */
    if (idx < 0) idx = 0;
    
    /* Multi-level array access with struct field */
    int val = arr[idx].x[i % 4] + arr[i].y;
    
    /* More complex addressing with different variables */
    val += arr[(i * v4 + offset) / scale].z[0];
    
    return val;
}

/* Helper to force complex addressing in stores */
NOOPT void complex_store(struct Data *arr, int i, int val, 
                         int v5, int v6, int v7, int v8) {
    /* Different complex index for store */
    int store_idx = (i * v5 + v6) % v7;
    
    /* Complex store address with struct field */
    arr[store_idx].x[0] = val * v8 + extra1;
    
    /* Another store with different addressing */
    arr[i].z[1] = (val + v5) * v6;
}

/* Main function with high register pressure */
NOOPT int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra1 + 4;
    int v5 = extra2 + 5;
    int v6 = n + 6;
    int v7 = stride * 2;
    int v8 = offset * 2;
    int v9 = scale * 2;
    int v10 = extra1 * 2;
    int v11 = extra2 * 2;
    int v12 = n * 2;
    int v13 = stride + offset;
    int v14 = offset + scale;
    int v15 = scale + extra1;
    int v16 = extra1 + extra2;
    int v17 = extra2 + n;
    int v18 = stride * scale;
    int v19 = offset * extra1;
    int v20 = scale * extra2;
    int v21 = v1 + v2;
    int v22 = v3 + v4;
    int v23 = v5 + v6;
    int v24 = v7 + v8;
    int v25 = v9 + v10;
    
    int sum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Use all variables in calculations to keep them live */
        int temp1 = v1 * i + v2;
        int temp2 = v3 * i + v4;
        int temp3 = v5 * i + v6;
        
        /* Complex load operation - triggers input address reloads */
        int loaded = complex_load(arr, i, 
                                 (temp1 + v7) / v8,  /* Complex expression */
                                 (temp2 + v9) % v10,
                                 (temp3 + v11) / v12,
                                 (v13 * i + v14) % v15);
        
        /* More calculations using many variables */
        int computed = loaded * v16 + v17 - v18 + v19 * v20;
        computed += v21 * i - v22 + v23 / (v24 + 1) + v25;
        
        /* Complex store operation - triggers output address reloads */
        complex_store(arr, i, computed,
                     (v1 + i) % v2,    /* Different complex expressions */
                     (v3 * i) / v4,
                     (v5 + i) % v6,
                     (v7 * i + v8) / v9);
        
        /* Inline assembly to force operand address reloads */
        int asm_idx = (i * v10 + v11) / v12;
        asm volatile("# %0" : : "m" (arr[asm_idx].y));
        
        /* Another inline assembly with different addressing */
        int asm_idx2 = (i * v13 + v14) % v15;
        asm volatile("# %0" : : "m" (arr[asm_idx2].z[0]));
        
        /* Use computed value to prevent dead code elimination */
        sum += arr[i].x[0] + arr[i].z[1];
        
        /* Update some variables to prevent optimization */
        v1 = (v1 + 1) % 17;
        v2 = (v2 + 3) % 19;
        v3 = (v3 + 5) % 23;
    }
    
    return sum;
}

/* Another helper with different addressing patterns */
NOOPT void mixed_addressing(struct Data *arr, int n, int *results) {
    for (int i = 0; i < n; ++i) {
        /* Mix array and pointer arithmetic */
        struct Data *ptr = arr + i;
        
        /* Complex addressing with pointer arithmetic */
        int idx1 = (i * stride + offset) / scale;
        int idx2 = (i * extra1 + extra2) % stride;
        
        /* Multiple memory accesses with different addressing modes */
        results[i] = ptr->x[0] 
                   + arr[idx1].y 
                   + arr[idx2].z[i % 2]
                   + (arr + idx1)->x[1]
                   + ptr[offset % 4].z[0];
        
        /* Inline assembly forcing address computation */
        asm volatile("# %0" : : "m" (ptr->x[2]));
        asm volatile("# %0" : : "m" ((arr + idx2)->y));
    }
}

int main() {
    /* Non-constant loop bound */
    volatile int N = 1024;
    int n = N;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    int *results = (int*)malloc(n * sizeof(int));
    
    if (!arr || !results) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 97;
        }
        arr[i].y = (i * 11) % 89;
        arr[i].z[0] = (i * 17) % 83;
        arr[i].z[1] = (i * 19) % 79;
    }
    
    /* Call functions that trigger complex reloads */
    int sum1 = complex_access_loop(arr, n);
    
    /* Another function with different addressing patterns */
    mixed_addressing(arr, n, results);
    
    /* Compute checksum to prevent optimization */
    int checksum = sum1;
    for (int i = 0; i < n; ++i) {
        checksum += results[i];
        checksum += arr[i].x[0] + arr[i].y + arr[i].z[0] + arr[i].z[1];
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    free(results);
    
    return 0;
}
