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
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int mod_val = 11;

/* Helper function to force complex addressing - marked noinline/noipa */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with different values to keep them live */
    v1 = stride;
    v2 = offset;
    v3 = scale;
    v4 = mod_val;
    v5 = n % 13;
    v6 = n % 17;
    v7 = n % 19;
    v8 = n % 23;
    v9 = n % 29;
    v10 = n % 31;
    v11 = n % 37;
    v12 = n % 41;
    v13 = n % 43;
    v14 = n % 47;
    v15 = n % 53;
    v16 = n % 59;
    v17 = n % 61;
    v18 = n % 67;
    v19 = n % 71;
    v20 = n % 73;
    v21 = n % 79;
    v22 = n % 83;
    v23 = n % 89;
    v24 = n % 97;
    v25 = n % 101;
    
    int checksum = 0;
    
    /* Main loop with complex addressing patterns */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + v3 * v4 - v5) / v6;
        idx = (idx + v7 * v8 - v9) % v10;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + 
                  arr[(i * v11 + v12) / v13 % n].y + 
                  arr[(i * v14 - v15) % n].z;
        
        /* Use more variables in computation to keep them live */
        val = val * v16 + v17 - v18 * v19 + v20;
        
        /* Complex store - triggers output address reloads */
        int store_idx = (i * v21 + v22 * v23 - v24) % v25;
        if (store_idx < 0) store_idx = -store_idx;
        store_idx = store_idx % n;
        
        /* Store with complex addressing mode */
        arr[store_idx].x[(i + v1) % 4] = val + v2 * v3 - v4;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v5 + v6) % n].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address %0" 
                     : 
                     : "m" (arr[store_idx].x[(i + v7) % 4]));
        
        /* Use all variables to keep them live */
        checksum += val + v8 + v9 * v10 - v11 + v12 % (v13 + 1) + 
                   v14 - v15 * v16 + v17 / (v18 + 1) + 
                   v19 % (v20 + 1) + v21 - v22 * v23 + 
                   v24 % (v25 + 1);
        
        /* Force spills by using all variables in conditional */
        if ((v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
             v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
             v21 + v22 + v23 + v24 + v25) % 7 == 0) {
            checksum += i;
        }
    }
    
    /* Final computation using all variables */
    checksum += v1 - v2 + v3 * v4 - v5 + v6 % (v7 + 1) + 
               v8 - v9 * v10 + v11 / (v12 + 1) + 
               v13 % (v14 + 1) + v15 - v16 * v17 + 
               v18 % (v19 + 1) + v20 - v21 * v22 + 
               v23 % (v24 + 1) + v25;
    
    return checksum;
}

/* Another function to trigger different reload contexts */
__attribute__((noinline, noipa))
void complex_store_operations(struct Data *arr, int n, int *output) {
    volatile int a = 5, b = 9, c = 13, d = 17;
    
    for (int i = 0; i < n; ++i) {
        /* Complex output addressing for stores */
        int out_idx = (i * a + b * c - d) % n;
        if (out_idx < 0) out_idx = -out_idx;
        
        /* Complex computation for store value */
        int val = arr[i].x[0] * a + arr[i].y * b - arr[i].z * c + d;
        
        /* Store with complex addressing - triggers output address reloads */
        output[out_idx] = val + i * (a + b - c + d);
        
        /* Inline assembly forcing address reload for output */
        asm volatile("# Output address %0" : : "m" (output[(i * b + c) % n]));
    }
}

int main() {
    volatile int N = 1024;
    int n = N;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    int *output = (int*)malloc(n * sizeof(int));
    
    if (!arr || !output) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pattern */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 97;
        }
        arr[i].y = (i * 11) % 101;
        arr[i].z = (i * 17) % 103;
        output[i] = 0;
    }
    
    /* Call functions that trigger complex reloads */
    int checksum1 = complex_access_loop(arr, n);
    complex_store_operations(arr, n, output);
    
    /* Compute final checksum */
    int final_checksum = checksum1;
    for (int i = 0; i < n; ++i) {
        final_checksum += output[i] + arr[i].x[0] + arr[i].y + arr[i].z;
    }
    
    printf("Checksum: %d\n", final_checksum);
    
    free(arr);
    free(output);
    
    return 0;
}
