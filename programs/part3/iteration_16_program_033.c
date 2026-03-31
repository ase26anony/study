#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of addressing calculations */
volatile int stride = 7;
volatile int offset = 13;
volatile int scale = 3;
volatile int mod_factor = 11;

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
};

/* Force no optimization/inlining */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, int init_val) {
    /* Declare many local variables to consume registers */
    int v1 = init_val;
    int v2 = init_val * 2;
    int v3 = init_val * 3;
    int v4 = init_val * 4;
    int v5 = init_val * 5;
    int v6 = init_val * 6;
    int v7 = init_val * 7;
    int v8 = init_val * 8;
    int v9 = init_val * 9;
    int v10 = init_val * 10;
    int v11 = init_val * 11;
    int v12 = init_val * 12;
    int v13 = init_val * 13;
    int v14 = init_val * 14;
    int v15 = init_val * 15;
    int v16 = init_val * 16;
    int v17 = init_val * 17;
    int v18 = init_val * 18;
    int v19 = init_val * 19;
    int v20 = init_val * 20;
    int v21 = init_val * 21;
    int v22 = init_val * 22;
    int v23 = init_val * 23;
    int v24 = init_val * 24;
    int v25 = init_val * 25;
    
    int sum = 0;
    
    /* Complex loop with multiple addressing modes */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 + offset) / scale;
        idx = (idx * v3 + v4) % mod_factor;
        
        /* Force RELOAD_FOR_INPUT_ADDRESS: complex addressing for load */
        int val1 = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* More complex calculations keeping variables live */
        int tmp1 = val1 * v5 + v6;
        int tmp2 = tmp1 / v7 + v8;
        int tmp3 = tmp2 * v9 - v10;
        
        /* Different complex index for output */
        int out_idx = (i * v11 + v12) / stride;
        out_idx = (out_idx * v13 + v14) % (mod_factor - 1);
        
        /* Force RELOAD_FOR_OUTPUT_ADDRESS: complex addressing for store */
        arr[out_idx].x[0] = tmp3 + v15;
        
        /* Even more complex addressing mixing struct fields */
        int idx2 = (i * v16 + v17) / 2;
        int val2 = arr[idx2].z + arr[(i * v18) % n].x[2];
        
        /* Use inline assembly to force RELOAD_FOR_OPERAND_ADDRESS */
        /* Memory constraint with complex addressing */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(idx * v19 + v20) % n].y)
                     : "memory");
        
        /* Another inline assembly with different addressing */
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[(out_idx * v21 + v22) % n].x[3])
                     : "memory");
        
        /* Mix in more calculations to keep variables live */
        v1 = v1 + v23;
        v2 = v2 + v24;
        v3 = v3 + v25;
        v4 = v4 + val1;
        v5 = v5 + val2;
        
        /* Complex store with addressing using multiple variables */
        int store_idx = (i * v6 + v7 + v8) / 5;
        arr[store_idx].y = v9 + v10 + v11;
        
        /* Force RELOAD_FOR_INPADDR_ADDRESS through nested addressing */
        int nested_idx = arr[arr[(i * v12) % n].x[1] % n].x[0];
        arr[i].z = nested_idx + v13;
        
        /* Accumulate sum to prevent dead code elimination */
        sum += arr[i].x[0] + arr[i].y + arr[i].z;
        
        /* Rotate variables to keep them all live */
        int rot = v14;
        v14 = v15; v15 = v16; v16 = v17; v17 = v18; v18 = v19;
        v19 = v20; v20 = v21; v21 = v22; v22 = v23; v23 = v24;
        v24 = v25; v25 = rot;
    }
    
    /* Use all variables in final calculation */
    sum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
           v21 + v22 + v23 + v24 + v25;
    
    return sum;
}

/* Another noinline function to force different reload contexts */
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
    /* Non-constant size to prevent optimization */
    volatile int array_size = 1024;
    int n = array_size;
    
    /* Allocate array with volatile pointer to prevent optimizations */
    struct Data *arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize array */
    init_array(arr, n);
    
    /* Perform complex accesses */
    int result = complex_access_loop(arr, n, 42);
    
    /* Additional complex access with different pattern */
    int result2 = 0;
    for (int i = 0; i < 100; ++i) {
        /* Force RELOAD_FOR_OUTADDR_ADDRESS with complex store addressing */
        int idx = (i * stride + offset * i) / (scale + i % 3);
        arr[idx % n].x[i % 4] = result + i;
        
        /* Force RELOAD_FOR_OTHER_ADDRESS through inline assembly */
        asm volatile("# Other address type %0" 
                     : "=m" (arr[(idx + i) % n].y)
                     : 
                     : "memory");
        
        result2 += arr[i % n].z;
    }
    
    /* Final computation using results */
    int final_result = result + result2;
    
    /* Print to prevent elimination */
    printf("Result: %d\n", final_result);
    
    free(arr);
    return 0;
}
