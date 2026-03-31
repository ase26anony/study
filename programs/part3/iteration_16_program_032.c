#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization from simplifying addressing */
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int extra1 = 11;
volatile int extra2 = 13;

struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Force complex addressing to remain in functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to exhaust registers */
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
    int v21 = v16 + v17;
    int v22 = v18 + v19;
    int v23 = v20 + v21;
    int v24 = v22 + v23;
    int v25 = v24 * 2;
    
    int sum = 0;
    
    /* Complex loop with mixed addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation */
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % (n - 1);
        if (idx < 0) idx = -idx;
        
        /* Complex load with struct field access - triggers RELOAD_FOR_INPUT_ADDRESS */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        val += arr[(idx * v6 + v7) / v8 % n].z[i % 2];
        
        /* More complex address calculations using many live variables */
        int idx2 = (i * v9 + v10) / v11;
        idx2 = (idx2 * v12 + v13) % (n - 1);
        if (idx2 < 0) idx2 = -idx2;
        
        int idx3 = (i * v14 + v15) / v16;
        idx3 = (idx3 * v17 + v18) % (n - 1);
        if (idx3 < 0) idx3 = -idx3;
        
        /* Complex store - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val * v19 + v20;
        
        /* Another store with different complex addressing */
        arr[idx2].y = arr[idx3].x[2] * v21 + v22;
        
        /* Store with even more complex addressing */
        int idx4 = (i * v23 + v24) / v25;
        idx4 = idx4 % (n - 1);
        if (idx4 < 0) idx4 = -idx4;
        
        arr[idx4].z[0] = arr[i].x[1] * v1 + arr[idx].y * v2;
        
        /* Inline assembly to force specific reload types */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand: %0" 
                     : 
                     : "m" (arr[(i * v3 + v4) / v5 % n].y));
        
        /* Another inline asm with different addressing */
        asm volatile("# Another address: %0" 
                     : 
                     : "m" (arr[idx2].x[(i * v6) % 4]));
        
        /* Mix in more variable usage to keep them all live */
        v1 = (v1 + 1) % 17;
        v2 = (v2 + 1) % 19;
        v3 = (v3 + 1) % 23;
        v4 = (v4 + 1) % 29;
        v5 = (v5 + 1) % 31;
        
        /* Update sum to prevent dead code elimination */
        sum += arr[i].x[0] + arr[idx].y + arr[idx2].z[0];
    }
    
    /* Use all variables in final calculation */
    int final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                v21 + v22 + v23 + v24 + v25;
    
    return sum + final;
}

__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z[0] = (i * 31 + 11) % 100;
        arr[i].z[1] = (i * 37 + 13) % 100;
    }
}

int main() {
    /* Use volatile to prevent constant propagation */
    volatile int N = 1024;
    
    /* Allocate array dynamically to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant pattern */
    init_array(arr, N);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, N);
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < N && i < 100; ++i) {
        checksum += arr[i].x[0] + arr[i].y;
    }
    
    free(arr);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    return 0;
}
