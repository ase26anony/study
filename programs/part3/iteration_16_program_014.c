#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 25

struct Data {
    int x[4];
    int y;
    int z;
};

/* Prevent optimization of complex addressing calculations */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int mod) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with different values to keep them live */
    v1 = stride + 1; v2 = offset + 2; v3 = scale + 3; v4 = mod + 4;
    v5 = v1 * 2; v6 = v2 * 3; v7 = v3 * 4; v8 = v4 * 5;
    v9 = v5 + v6; v10 = v7 - v8; v11 = v9 * v10; v12 = v11 / 7;
    v13 = v12 + v1; v14 = v13 - v2; v15 = v14 * v3;
    v16 = v15 + v4; v17 = v16 / 2; v18 = v17 * 3;
    v19 = v18 - v5; v20 = v19 + v6; v21 = v20 * v7;
    v22 = v21 - v8; v23 = v22 / 9; v24 = v23 + v9;
    v25 = v24 - v10;
    
    int sum = 0;
    
    /* Loop with complex addressing that requires multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v3 - v4) / (v5 % scale + 1);
        idx = (idx * v6 + v7) / (v8 % mod + 1);
        idx = (idx + v9 - v10) * (v11 % 13 + 1);
        idx = idx % n;
        
        /* Complex addressing for LOAD (input address reloads) */
        int val1 = arr[(i * v12 + v13) / (v14 % scale + 1)].x[i % 4];
        int val2 = arr[(idx * v15 + v16) / (v17 % mod + 1)].y;
        int val3 = arr[(val1 * v18 + v19) / (v20 % scale + 1)].z;
        
        /* Use all variables in computation to keep them live */
        int complex_val = val1 * v21 + val2 * v22 - val3 * v23 
                         + v24 * i - v25 * idx;
        
        /* Complex addressing for STORE (output address reloads) */
        int store_idx = (i * v1 + v2 * v3 - v4) / (v5 % scale + 1);
        store_idx = (store_idx + v6 * v7 - v8) % n;
        
        /* Store with complex addressing - triggers output address reloads */
        arr[store_idx].x[(i + v9) % 4] = complex_val + v10;
        arr[(store_idx * v11 + v12) / (v13 % mod + 1)].y = 
            complex_val * v14 - v15;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Operand address constraint %0" 
                    : 
                    : "m" (arr[(idx * v16 + v17) / (v18 % scale + 1)].x[0]));
        
        /* Another inline assembly with different addressing */
        asm volatile("# Other operand address %0" 
                    : 
                    : "m" (arr[(i * v19 + v20) / (v21 % mod + 1)].z));
        
        /* Use inline assembly that modifies memory with complex address */
        /* This may trigger RELOAD_FOR_OUTADDR_ADDRESS */
        int temp = complex_val + v22;
        asm volatile("# Memory output %0" 
                    : "=m" (arr[(temp * v23 + v24) / (v25 % scale + 1)].x[2]));
        
        /* Mix in some pointer arithmetic that requires address reloads */
        struct Data *ptr1 = &arr[(i * v1 + v2) / (v3 % mod + 1)];
        struct Data *ptr2 = &arr[(idx * v4 + v5) / (v6 % scale + 1)];
        
        /* Access through pointers with offset calculations */
        ptr1->x[(i + v7) % 4] = ptr2->y * v8 + v9;
        ptr2->z = ptr1->x[(idx + v10) % 4] * v11 - v12;
        
        /* Keep sum to prevent dead code elimination */
        sum += complex_val + ptr1->x[0] + ptr2->z;
        
        /* Update some variables to create dependencies */
        v1 = (v1 + i) % 100;
        v2 = (v2 + idx) % 100;
        v3 = (v3 + complex_val) % 100;
        v4 = (v4 + store_idx) % 100;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 100;
        arr[i].z = (i * 17) % 100;
    }
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int mod = 5;
    
    /* Allocate array dynamically to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize array */
    init_array(arr, N);
    
    /* Perform complex accesses that trigger various reload types */
    int result = complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z;
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
