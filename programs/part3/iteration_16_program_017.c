#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 3;
volatile int scale = 2;
volatile int mod_val = 100;

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with different values to keep them live */
    v1 = stride;
    v2 = offset;
    v3 = scale;
    v4 = mod_val;
    v5 = n % 7;
    v6 = n % 11;
    v7 = n % 13;
    v8 = n % 17;
    v9 = n % 19;
    v10 = n % 23;
    v11 = v1 + v2;
    v12 = v3 + v4;
    v13 = v5 + v6;
    v14 = v7 + v8;
    v15 = v9 + v10;
    v16 = v11 * v12;
    v17 = v13 * v14;
    v18 = v15 * v16;
    v19 = v17 * v18;
    v20 = v19 % 97;
    v21 = v20 + 1;
    v22 = v21 * 3;
    v23 = v22 / 5;
    v24 = v23 + 7;
    v25 = v24 - 11;
    
    int checksum = 0;
    
    /* Main loop with complex addressing patterns */
    for (int i = 1; i < n; ++i) {
        /* Force RELOAD_FOR_INPUT_ADDRESS: complex addressing for load */
        /* Multi-component address calculation that can't fit in one instruction */
        int idx = (i * v1 + v2 + v3 * v4 - v5) / v6;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex addressing for input (load) - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* Use more variables in computation to keep them live */
        val = val * v7 + v8 - v9 * v10 + v11;
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS: complex addressing for store */
        /* Different complex address for output */
        int out_idx = (i * v12 + v13 * v14 - v15) % n;
        if (out_idx < 0) out_idx = -out_idx;
        
        /* Complex addressing for output (store) - triggers output address reloads */
        arr[out_idx].x[(i + v16) % 4] = val + v17 - v18 * v19;
        
        /* Force RELOAD_FOR_OPERAND_ADDRESS with inline assembly */
        /* Memory constraint with complex addressing */
        asm volatile("# Complex address operand: %0" 
                     : 
                     : "m" (arr[(idx + v20) % n].y)
                     : "memory");
        
        /* Another inline assembly with different addressing for OPADDR_ADDR */
        int temp_idx = (i * v21 + v22) / (v23 + 1);
        temp_idx = temp_idx % n;
        if (temp_idx < 0) temp_idx = -temp_idx;
        
        asm volatile("# Another address: %0\n\t"
                     "nop"
                     : 
                     : "m" (arr[temp_idx].z),
                       "r" (v24), "r" (v25)
                     : "memory");
        
        /* Mix in more complex addressing for other reload types */
        /* RELOAD_FOR_INPADDR_ADDRESS might be triggered here */
        int complex_val = arr[(i * v24 + v25) % n].x[(v1 * i) % 4] 
                        + arr[(i + v2) % n].y 
                        - arr[(i * v3) % n].z;
        
        /* Use the value to prevent elimination */
        checksum += complex_val + val + i;
        
        /* Update some variables to create dependencies */
        v1 = (v1 + 1) % 13;
        v2 = (v2 + i) % 17;
        v3 = (v3 * 3) % 19;
        v4 = (v4 + v5) % 23;
        v5 = (v5 + v6) % 29;
    }
    
    /* Final complex computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10
              + v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20
              + v21 + v22 + v23 + v24 + v25;
    
    return checksum;
}

/* Another noinline function to force more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 1000;
        }
        arr[i].y = (i * 31) % 1000;
        arr[i].z = (i * 53) % 1000;
    }
}

int main() {
    /* Use volatile to prevent constant propagation of array size */
    volatile int array_size = 1024;
    int n = array_size;
    
    /* Allocate array dynamically to avoid static addressing */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant pattern */
    init_array(arr, n);
    
    /* Perform complex accesses that should trigger various reload types */
    int result = complex_access_loop(arr, n);
    
    /* Use result to prevent dead code elimination */
    printf("Checksum result: %d\n", result);
    
    /* Additional complex addressing in main to potentially trigger more reloads */
    int sum = 0;
    volatile int idx1 = 17, idx2 = 23, idx3 = 37;
    
    for (int i = 0; i < 100; ++i) {
        /* Mixed addressing modes */
        sum += arr[(i * idx1) % n].x[(i + idx2) % 4]
             - arr[(i + idx3) % n].y
             + arr[i % n].z;
    }
    
    printf("Additional sum: %d\n", sum + result);
    
    free(arr);
    return 0;
}
