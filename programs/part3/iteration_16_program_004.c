#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Structure to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static int complex_access_loop(struct Data *arr, int n, 
                               volatile int stride, 
                               volatile int offset, 
                               volatile int scale) {
    /* Declare many local variables to consume registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with non-trivial values */
    v1 = stride + 1;
    v2 = offset + 2;
    v3 = scale + 3;
    v4 = stride * 2;
    v5 = offset * 3;
    v6 = scale * 4;
    v7 = stride + offset;
    v8 = stride - scale;
    v9 = offset * scale;
    v10 = stride / 2;
    v11 = offset % 7;
    v12 = scale + stride;
    v13 = stride * 3 + 1;
    v14 = offset * 2 - 1;
    v15 = scale + 5;
    v16 = stride + offset + scale;
    v17 = stride * offset;
    v18 = scale * 2;
    v19 = stride % 11;
    v20 = offset % 13;
    v21 = scale % 17;
    v22 = v1 + v2 + v3;
    v23 = v4 * v5 - v6;
    v24 = v7 + v8 + v9;
    v25 = v10 * v11 / (v12 + 1);
    
    int sum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v3 * i / (v4 + 1)) / (v5 + v6 % (v7 + 1));
        idx = (idx + v8 - v9 * (i % (v10 + 1))) % n;
        if (idx < 0) idx = -idx;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + 
                  arr[(i-1) * v11 / (v12 + 1)].y + 
                  arr[(idx + v13) % n].z[i % 2];
        
        /* More complex calculations keeping variables live */
        val = val * v14 + v15 - v16 * (i % (v17 + 1)) / (v18 + 1);
        val += v19 * v20 - v21 * v22 + v23 / (v24 + 1) - v25;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[(v1 + i) % 4] = val + 
                                 arr[(idx + v2) % n].x[(v3 + i) % 4] * 
                                 arr[(i + v4) % n].y;
        
        /* Another complex store with different addressing */
        /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        arr[(i * v5 + v6) % n].z[i % 2] = 
            arr[idx].x[(i + v7) % 4] * v8 + v9 * i - v10;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" 
                     : 
                     : "m" (arr[(idx * v11 + v12) % n].y));
        
        asm volatile("# Complex address 2: %0" 
                     : 
                     : "m" (arr[(i * v13 + v14 * v15) % n].x[(v16 + i) % 4]));
        
        /* More complex addressing in inline asm */
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address 3: %0" 
                     : 
                     : "m" (arr[((i * v17 + v18) / (v19 + 1)) % n].z[(v20 + i) % 2]));
        
        /* Keep sum for result */
        sum += val + arr[i].x[0] + arr[idx].y;
        
        /* Modify some variables to prevent optimization */
        v1 = (v1 + i) % 100;
        v2 = (v2 * 3 + 1) % 100;
        v3 = (v3 + v4) % 100;
        v4 = (v4 * 5 - 2) % 100;
        v5 = (v5 + v6 * 2) % 100;
    }
    
    return sum;
}

__attribute__((noinline, noipa))
static void initialize_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 1000;
        }
        arr[i].y = (i * 23 + 7) % 1000;
        arr[i].z[0] = (i * 31 + 11) % 1000;
        arr[i].z[1] = (i * 37 + 17) % 1000;
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int array_size = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Allocate array dynamically to prevent stack optimization */
    struct Data *arr = (struct Data*)malloc(array_size * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with non-constant pattern */
    initialize_array(arr, array_size);
    
    /* Call the complex access function */
    int result = complex_access_loop(arr, array_size, stride, offset, scale);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex computation using the array */
    int checksum = 0;
    for (int i = 0; i < array_size; ++i) {
        /* More complex addressing */
        checksum += arr[(i * stride + offset) / (scale + 1)].x[i % 4];
        checksum -= arr[i].y * ((i + offset) % 10);
        checksum += arr[(i + stride) % array_size].z[i % 2];
    }
    printf("Checksum: %d\n", checksum);
    
    free(arr);
    return 0;
}
