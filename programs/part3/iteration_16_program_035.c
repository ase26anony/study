#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 20

struct Data {
    int x[4];
    int y;
};

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n, 
                        volatile int stride, volatile int offset, 
                        volatile int scale, volatile int extra) {
    /* Declare many local variables to consume registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = extra + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = extra * 2;
    int v9 = stride + offset;
    int v10 = scale + extra;
    int v11 = stride * offset;
    int v12 = scale * extra;
    int v13 = stride - offset;
    int v14 = scale - extra;
    int v15 = stride / 2;
    int v16 = offset / 2;
    int v17 = scale / 2;
    int v18 = extra / 2;
    int v19 = (stride + offset) * scale;
    int v20 = (scale + extra) * stride;
    
    /* Force all variables to be live through the loop */
    for (int i = 1; i < n; ++i) {
        /* Complex addressing for LOAD (input address reloads) */
        /* RELOAD_FOR_INPUT_ADDRESS, RELOAD_FOR_INPADDR_ADDRESS */
        int idx1 = (i * v1 + v2) / v3;
        int idx2 = (i * v4 + v5) / v6;
        
        /* Mix struct fields and array indices */
        int val1 = arr[idx1].x[i % 4] + arr[i-1].y;
        int val2 = arr[idx2].x[(i+1) % 4] + arr[i-1].x[0];
        
        /* Complex addressing for STORE (output address reloads) */
        /* RELOAD_FOR_OUTPUT_ADDRESS, RELOAD_FOR_OUTADDR_ADDRESS */
        int store_idx1 = (i * v7 + v8) / v9;
        int store_idx2 = (i * v10 + v11) / v12;
        
        arr[store_idx1].x[0] = val1 * v13 + v14;
        arr[store_idx2].x[1] = val2 * v15 + v16;
        
        /* More complex addressing with multiple components */
        /* RELOAD_FOR_OPERAND_ADDRESS, RELOAD_FOR_OPADDR_ADDR */
        int idx3 = ((i * v17 + v18) * v19) / v20;
        arr[idx3].x[2] = arr[idx3].x[3] * v1 + v2;
        
        /* Use inline assembly to force specific address reloads */
        /* RELOAD_FOR_OTHER_ADDRESS, RELOAD_OTHER */
        asm volatile("# Complex address 1: %0" : : "m" (arr[(i * v3 + v4) / v5].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[(i * v6 + v7) / v8].x[0]));
        
        /* Keep all variables live by using them in expressions */
        v1 = v1 + v2;
        v2 = v2 + v3;
        v3 = v3 + v4;
        v4 = v4 + v5;
        v5 = v5 + v6;
        v6 = v6 + v7;
        v7 = v7 + v8;
        v8 = v8 + v9;
        v9 = v9 + v10;
        v10 = v10 + v11;
        v11 = v11 + v12;
        v12 = v12 + v13;
        v13 = v13 + v14;
        v14 = v14 + v15;
        v15 = v15 + v16;
        v16 = v16 + v17;
        v17 = v17 + v18;
        v18 = v18 + v19;
        v19 = v19 + v20;
        v20 = v20 + i;
    }
    
    /* Use all variables one more time to prevent dead code elimination */
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5),
                   "r"(v6), "r"(v7), "r"(v8), "r"(v9), "r"(v10),
                   "r"(v11), "r"(v12), "r"(v13), "r"(v14), "r"(v15),
                   "r"(v16), "r"(v17), "r"(v18), "r"(v19), "r"(v20));
}

__attribute__((noinline, noipa))
int compute_checksum(struct Data *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        sum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3] + arr[i].y;
    }
    return sum;
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 11;
    
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with deterministic pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 31 + j * 7) % 100;
        }
        arr[i].y = (i * 17) % 100;
    }
    
    /* Call the function that triggers complex reloads */
    complex_access_loop(arr, N, stride, offset, scale, extra);
    
    /* Compute and print checksum to prevent elimination */
    int checksum = compute_checksum(arr, N);
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
