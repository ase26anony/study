#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Structure with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod1 = 5;
volatile int mod2 = 11;

/* Helper function to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to exhaust registers */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5;
    int v6 = 6, v7 = 7, v8 = 8, v9 = 9, v10 = 10;
    int v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    int sum = 0;
    
    /* Complex loop with many live variables and addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Use all variables in complex expressions to keep them live */
        v1 = v1 * mod1 + i;
        v2 = v2 * mod2 + v1;
        v3 = v3 + v2 / mod1;
        v4 = v4 ^ v3;
        v5 = v5 + v4 * v1;
        v6 = v6 - v5;
        v7 = v7 * v6 + v2;
        v8 = v8 / (v7 + 1);
        v9 = v9 ^ v8;
        v10 = v10 + v9 * v3;
        
        /* Complex addressing for LOAD (input address reloads) */
        /* arr[(i * stride + offset) / scale].x[i % 4] */
        int idx1 = (i * stride + offset) / scale;
        int idx2 = (i * v1 + v2) % 4;  /* Use runtime variables */
        int idx3 = (v3 * i + v4) % n;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS with complex array access */
        int val1 = arr[idx1].x[idx2];
        
        /* More complex addressing mixing struct fields */
        /* arr[(v5 * i + v6) / (v7 + 1)].y */
        int idx4 = (v5 * i + v6) / (v7 + 1);
        int val2 = arr[idx4].y;
        
        /* Complex addressing with multiple calculations */
        /* arr[((v8 * i + v9) ^ v10) % n].z[i % 2] */
        int idx5 = ((v8 * i + v9) ^ v10) % n;
        int val3 = arr[idx5].z[i % 2];
        
        /* Combine values using more variables */
        int combined = val1 * v11 + val2 * v12 + val3 * v13;
        
        /* Complex addressing for STORE (output address reloads) */
        /* arr[(v14 * i + v15) % n].x[(v16 + i) % 4] = ... */
        int store_idx1 = (v14 * i + v15) % n;
        int store_idx2 = (v16 + i) % 4;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS */
        arr[store_idx1].x[store_idx2] = combined + v17;
        
        /* Different store with more complex addressing */
        int store_idx3 = (v18 * i * stride + v19 * offset) / (scale + v20);
        arr[store_idx3].y = (val1 + val2) * v21;
        
        /* Inline assembly to force operand address reloads */
        /* Force RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile("# Operand address: %0" : : "m" (arr[idx1].y));
        
        /* Force RELOAD_FOR_OTHER_ADDRESS with different addressing */
        asm volatile("# Other address: %0" : : "m" (arr[store_idx3].z[0]));
        
        /* More inline assembly with complex addressing */
        int idx6 = (v22 * i + v23) % n;
        asm volatile("# Input address: %0" : : "m" (arr[idx6].x[(v24 + i) % 4]));
        
        /* Use remaining variables */
        v11 = v11 + v25;
        v12 = v12 ^ v11;
        v13 = v13 * v12;
        v14 = v14 + v13;
        v15 = v15 - v14;
        v16 = v16 * v15;
        v17 = v17 / (v16 + 1);
        v18 = v18 ^ v17;
        v19 = v19 + v18 * v1;
        v20 = v20 - v19;
        v21 = v21 * v20;
        v22 = v22 + v21;
        v23 = v23 ^ v22;
        v24 = v24 * v23;
        v25 = v25 + v24;
        
        sum += arr[i].x[0];
    }
    
    /* Use all variables one more time to prevent dead code elimination */
    int final = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                v21 + v22 + v23 + v24 + v25;
    
    return sum + final;
}

/* Another noinline function to force more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 3) % 100;
        }
        arr[i].y = (i * 11) % 100;
        arr[i].z[0] = (i * 13) % 100;
        arr[i].z[1] = (i * 17) % 100;
    }
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = 1024;
    int actual_n = N;
    
    /* Allocate array with volatile pointer to prevent optimizations */
    struct Data *arr = (struct Data*)malloc(actual_n * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize array */
    init_array(arr, actual_n);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, actual_n);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < actual_n; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z[0] + arr[i].z[1];
    }
    
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
