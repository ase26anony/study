#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024

/* Struct with multiple fields to create complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Volatile variables to prevent constant propagation */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod1 = 5, mod2 = 11, mod3 = 17;

/* Helper functions marked to prevent optimization */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 7919 + j * 65537) % 10007;
        }
        arr[i].y = (i * 1009) % 10007;
        arr[i].z = (i * 1013) % 10007;
    }
}

/* Main function that triggers complex reloads */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    /* Many live variables to consume registers */
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = mod1;
    int v5 = mod2;
    int v6 = mod3;
    int v7 = n * 2;
    int v8 = n / 2;
    int v9 = v1 + v2;
    int v10 = v3 * v4;
    int v11 = v5 - v6;
    int v12 = v7 % v8;
    int v13 = v9 ^ v10;
    int v14 = v11 | v12;
    int v15 = v13 & v14;
    int v16 = v15 + 1;
    int v17 = v16 * 3;
    int v18 = v17 / 5;
    int v19 = v18 % 7;
    int v20 = v19 + 11;
    
    int sum = 0;
    
    /* Loop with complex addressing modes */
    for (int i = 1; i < n - 1; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) / v3 + v4) % v5;
        if (idx < 0) idx = -idx;
        idx = idx % (n - 2);
        
        /* More intermediate calculations to keep variables live */
        int t1 = v6 + i;
        int t2 = v7 - i;
        int t3 = v8 * i;
        int t4 = v9 / (i + 1);
        int t5 = v10 % (i + 2);
        
        /* Complex load with struct field access and array indexing */
        int field_idx = (i * v11 + v12) % 4;
        if (field_idx < 0) field_idx = -field_idx;
        
        /* RELOAD_FOR_INPUT_ADDRESS: complex addressing for load */
        int val = arr[idx].x[field_idx] 
                + arr[i-1].y 
                + arr[(i * v13 + v14) % n].z;
        
        /* More calculations using live variables */
        val = val * v15 + v16;
        val = (val + v17) / (v18 + 1);
        val = val ^ v19 | v20;
        
        /* Complex store addressing - RELOAD_FOR_OUTPUT_ADDRESS */
        int store_idx = ((i * t1 + t2) / (t3 + 1) + t4) % t5;
        if (store_idx < 0) store_idx = -store_idx;
        store_idx = store_idx % (n - 1);
        
        /* RELOAD_FOR_OUTPUT_ADDRESS: complex addressing for store */
        arr[store_idx].x[i % 4] = val + arr[i].x[0];
        
        /* Inline assembly to force address reloads */
        /* RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        int asm_idx = (i * v1 + v3) % n;
        asm volatile("# Complex address operand %0" 
                    : 
                    : "m" (arr[asm_idx].y), "m" (arr[(i + v2) % n].z));
        
        /* Another inline assembly with different addressing */
        /* RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address %0 %1"
                    :
                    : "m" (arr[(i * v4 + v5) % n].x[0]),
                      "m" (arr[(i * v6 + v7) % n].x[1]));
        
        /* RELOAD_FOR_INPADDR_ADDRESS: address of input address */
        int* ptr = &arr[(i * v8 + v9) % n].x[2];
        asm volatile("# Input address address %0" : : "r" (ptr));
        
        /* RELOAD_FOR_OUTADDR_ADDRESS: address of output address */
        int* out_ptr = &arr[(i * v10 + v11) % n].x[3];
        asm volatile("# Output address address %0" : : "r" (out_ptr));
        
        /* Update checksum */
        sum += arr[i].x[0] + arr[i].y + arr[i].z;
        
        /* Modify some variables to prevent optimization */
        v1 = (v1 + 1) % 17;
        v2 = (v2 + 3) % 19;
        v3 = (v3 + 5) % 23;
    }
    
    return sum;
}

/* Another function to create different reload contexts */
__attribute__((noinline, noipa))
int nested_complex_access(struct Data *arr, int n, int *indices) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        /* Very complex nested addressing */
        int idx1 = indices[(i * stride + offset) % n];
        int idx2 = indices[(i * scale + mod1) % n];
        int idx3 = indices[(i * mod2 + mod3) % n];
        
        /* Multiple complex memory accesses in one expression */
        total += arr[idx1].x[0] 
               * arr[idx2].x[1] 
               + arr[idx3].x[2] 
               - arr[(idx1 + idx2) % n].x[3]
               + arr[(idx2 * idx3) % n].y
               * arr[(idx1 * 3 + idx2 * 5 + idx3 * 7) % n].z;
    }
    
    return total;
}

int main() {
    /* Non-constant size to prevent optimization */
    volatile int N = SIZE;
    int actual_n = N;
    
    /* Allocate and initialize array */
    struct Data *arr = (struct Data*)malloc(actual_n * sizeof(struct Data));
    if (!arr) return 1;
    
    init_array(arr, actual_n);
    
    /* Create index array for complex addressing */
    int *indices = (int*)malloc(actual_n * sizeof(int));
    if (!indices) {
        free(arr);
        return 1;
    }
    
    for (int i = 0; i < actual_n; i++) {
        indices[i] = (i * 1103) % actual_n;
    }
    
    /* Call functions that trigger complex reloads */
    int sum1 = complex_access_loop(arr, actual_n);
    int sum2 = nested_complex_access(arr, actual_n, indices);
    
    /* Compute final result to prevent dead code elimination */
    int result = sum1 + sum2;
    
    /* Use result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Clean up */
    free(indices);
    free(arr);
    
    return 0;
}
