#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

/* Complex struct to force multi-level addressing */
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

/* Helper function with attributes to prevent optimization */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Declare many local variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod1;
    int v5 = mod2;
    int v6 = n % 7;
    int v7 = n % 11;
    int v8 = n % 13;
    int v9 = n % 17;
    int v10 = n % 19;
    int v11 = n % 23;
    int v12 = n % 29;
    int v13 = n % 31;
    int v14 = n % 37;
    int v15 = n % 41;
    int v16 = n % 43;
    int v17 = n % 47;
    int v18 = n % 53;
    int v19 = n % 59;
    int v20 = n % 61;
    
    int sum = 0;
    
    /* Loop with complex addressing in both loads and stores */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) / v3 + v4) % v5;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        if (idx == 0) idx = 1;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val1 = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* More complex calculation using many variables */
        int val2 = val1 * v6 + v7 - v8 + v9 * v10;
        int val3 = val2 / (v11 + 1) + v12 - v13 * v14;
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val3 + v15 - v16 + v17 * v18;
        
        /* Even more complex addressing for another store */
        /* Mixing different struct fields and calculations */
        int idx2 = (i * v19 + v20) % (n - 1);
        if (idx2 == 0) idx2 = 1;
        
        /* Store with complex addressing - different output address type */
        arr[idx2].z[i % 2] = val3 * v1 + v2 - v3 * v4;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[idx2].x[2]));
        asm volatile("# Complex address 3: %0" : : "m" (arr[i].z[1]));
        
        /* Use all variables in computation to keep them live */
        sum += arr[i].x[0] + arr[idx].y + arr[idx2].z[0] + 
               v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
               v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    }
    
    return sum;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23 + 7) % 100;
        arr[i].z[0] = (i * 29 + 3) % 100;
        arr[i].z[1] = (i * 31 + 11) % 100;
    }
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = ARRAY_SIZE;
    int actual_N = N;
    
    /* Allocate array with complex struct type */
    struct Data *arr = (struct Data*)malloc(actual_N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with pattern */
    init_array(arr, actual_N);
    
    /* Perform complex accesses that should trigger various reload types */
    int result = complex_access_loop(arr, actual_N);
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < actual_N; ++i) {
        checksum += arr[i].x[0] + arr[i].y + arr[i].z[0] + arr[i].z[1];
    }
    
    /* Use result and checksum to prevent optimization */
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
