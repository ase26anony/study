#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 20

/* Prevent optimizations that would simplify addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod_factor = 5;

/* Force complex addressing with no optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    /* Initialize with values derived from parameters to keep them live */
    v1 = n + 1; v2 = n + 2; v3 = n + 3; v4 = n + 4; v5 = n + 5;
    v6 = n + 6; v7 = n + 7; v8 = n + 8; v9 = n + 9; v10 = n + 10;
    v11 = n + 11; v12 = n + 12; v13 = n + 13; v14 = n + 14; v15 = n + 15;
    v16 = n + 16; v17 = n + 17; v18 = n + 18; v19 = n + 19; v20 = n + 20;
    
    int checksum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 * stride + offset) / (v3 + scale);
        idx = (idx * v4 + v5) % (v6 + mod_factor);
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % n;
        
        /* Complex load with struct field access - triggers input address reloads */
        int val = arr[idx].x[i % 4] + arr[i-1].y * v7;
        val += arr[(i + v8) % n].z * v9;
        
        /* More complex calculations using many variables */
        val = val * v10 + v11;
        val = (val + v12) / (v13 + 1);
        val = val ^ v14 ^ v15;
        
        /* Complex store - triggers output address reloads */
        int store_idx = (i * v16 + v17) % n;
        arr[store_idx].x[(i + v18) % 4] = val * v19 + v20;
        
        /* Inline assembly to force operand address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address operand: %0" 
                     : 
                     : "m" (arr[(idx + v1) % n].y));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address: %0" 
                     : 
                     : "m" (arr[store_idx].x[(i + 2) % 4]));
        
        /* Mix in some pointer arithmetic */
        struct Data *ptr1 = &arr[(i + v2) % n];
        struct Data *ptr2 = &arr[(i + v3) % n];
        
        /* Complex addressing through pointers */
        int val2 = ptr1->x[(i + v4) % 4] + ptr2->y * v5;
        
        /* Store with pointer-based addressing */
        ptr1->z = val2 * v6 + v7;
        
        /* Update checksum to prevent dead code elimination */
        checksum += val + val2 + arr[i].x[0];
        
        /* Modify some variables to keep them live and changing */
        v1 = (v1 + i) & 0xFF;
        v2 = (v2 * 3 + 1) & 0xFF;
        v3 = (v3 ^ i) & 0xFF;
        v4 = (v4 + v5) & 0xFF;
        v5 = (v5 * 5 + 7) & 0xFF;
    }
    
    return checksum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z = (i * 29 + 11) % 100;
    }
}

int main() {
    /* Use volatile to prevent constant propagation of array size */
    volatile int array_size = 1024;
    int n = array_size;
    
    /* Allocate array dynamically to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with pattern */
    init_array(arr, n);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, n);
    
    /* Additional complex operations in main to create more reload contexts */
    int extra_vars[NUM_VARS];
    for (int i = 0; i < NUM_VARS; ++i) {
        extra_vars[i] = i * 7 + 3;
    }
    
    /* More complex addressing in main */
    int sum = 0;
    for (int i = 0; i < 100; ++i) {
        int idx = (i * extra_vars[0] + extra_vars[1]) % n;
        sum += arr[idx].x[i % 4] * extra_vars[i % NUM_VARS];
        
        /* Store with complex addressing */
        int store_idx = (i * extra_vars[2] + extra_vars[3]) % n;
        arr[store_idx].y = sum % 1000;
    }
    
    result += sum;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    free(arr);
    return 0;
}
