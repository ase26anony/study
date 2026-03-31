#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 20

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n, 
                         volatile int stride, volatile int offset, 
                         volatile int scale, volatile int extra);

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Helper to force address computations to stay in code */
__attribute__((noinline, noipa))
void complex_access_loop(struct Data *arr, int n,
                         volatile int stride, volatile int offset,
                         volatile int scale, volatile int extra) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize with non-constant values to prevent optimization */
    v1 = stride + 1;
    v2 = offset + 2;
    v3 = scale + 3;
    v4 = extra + 4;
    v5 = stride * 2;
    v6 = offset * 3;
    v7 = scale * 4;
    v8 = extra * 5;
    v9 = stride + offset;
    v10 = scale + extra;
    v11 = stride * scale;
    v12 = offset * extra;
    v13 = v1 + v2;
    v14 = v3 + v4;
    v15 = v5 + v6;
    v16 = v7 + v8;
    v17 = v9 + v10;
    v18 = v11 + v12;
    v19 = v13 + v14;
    v20 = v15 + v16;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx + v4) * v5;
        idx = (idx % n + v6) & (n - 1);
        
        /* Even more complex index for output */
        int out_idx = (i * v7 + v8) / v9;
        out_idx = (out_idx + v10) * v11;
        out_idx = (out_idx % n + v12) & (n - 1);
        
        /* Force RELOAD_FOR_INPUT_ADDRESS: complex addressing in load */
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += arr[(i * v13 + v14) / v15].z;
        
        /* Use more variables in computation to keep them live */
        val = val * v16 + v17 - v18 + v19 * v20;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS: complex addressing in store */
        arr[out_idx].x[0] = val;
        arr[out_idx].x[1] = val + v1;
        arr[out_idx].x[2] = val + v2;
        arr[out_idx].x[3] = val + v3;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS with inline assembly */
        /* Memory constraint with complex addressing */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[idx].y), "m" (arr[out_idx].z)
                     : "memory");
        
        /* Another inline asm with different addressing for OPADDR_ADDR */
        int temp = arr[(i * v4 + v5) / v6].x[0];
        asm volatile("# Another address %0" 
                     : "=r" (temp)
                     : "0" (temp), "m" (arr[(i * v7 + v8) / v9].y)
                     : "cc");
        
        /* Mix in more complex addressing with struct fields */
        arr[i].y = arr[(i * v10 + v11) / v12].x[(i + v13) % 4] 
                   + arr[(i * v14 + v15) / v16].x[(i + v17) % 4];
        
        /* Force spill/reload by using all variables */
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
}

/* Another helper to force different reload contexts */
__attribute__((noinline, noipa))
void mixed_address_operations(struct Data *arr, int n, 
                              volatile int p1, volatile int p2) {
    int t1 = p1 + 1;
    int t2 = p2 + 2;
    int t3 = t1 * t2;
    int t4 = t2 * t1;
    
    for (int i = 0; i < n; i++) {
        /* Force RELOAD_FOR_INPADDR_ADDRESS */
        int idx1 = (i * t1 + t2) / t3;
        int idx2 = (i * t3 + t4) / t1;
        
        /* Complex addressing in both source and destination */
        arr[idx1].x[0] = arr[idx2].x[1] + arr[i].y;
        
        /* Force RELOAD_FOR_OUTADDR_ADDRESS with pointer arithmetic */
        struct Data *ptr = &arr[(i * t2 + t3) / t4];
        ptr->z = ptr->x[0] * ptr->x[1];
        
        /* Use inline asm with memory operands */
        asm volatile("# Mixed addresses %0 %1"
                     :
                     : "m" (arr[idx1].y), "m" (ptr->z)
                     : "memory");
    }
}

int main() {
    volatile int N = 1024;  /* volatile to prevent constant propagation */
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int extra = 11;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 23) % 100;
        }
        arr[i].y = (i * 31) % 100;
        arr[i].z = (i * 47) % 100;
    }
    
    /* Call functions that trigger complex reloads */
    complex_access_loop(arr, N, stride, offset, scale, extra);
    mixed_address_operations(arr, N / 2, stride, offset);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3];
        checksum += arr[i].y + arr[i].z;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
