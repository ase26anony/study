#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 1024

struct Data {
    int x[4];
    int y;
};

/* Prevent optimization of complex addressing calculations */
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n, 
                       volatile int stride, volatile int offset, 
                       volatile int scale, volatile int mod) {
    /* Many live variables to exhaust registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = mod + 4;
    int v5 = stride * 2;
    int v6 = offset * 2;
    int v7 = scale * 2;
    int v8 = mod * 2;
    int v9 = stride + offset;
    int v10 = scale + mod;
    int v11 = stride * offset;
    int v12 = scale * mod;
    int v13 = v1 + v2;
    int v14 = v3 + v4;
    int v15 = v5 + v6;
    int v16 = v7 + v8;
    int v17 = v9 + v10;
    int v18 = v11 + v12;
    int v19 = v13 + v14;
    int v20 = v15 + v16;
    
    int sum = 0;
    
    /* Complex loop with addressing that requires multiple reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation - forces address computation in steps */
        int idx = (i * v1 + v2) / v3;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Complex load addressing - triggers RELOAD_FOR_INPUT_ADDRESS */
        /* arr[idx].x[i % 4] uses struct field + array index + variable index */
        int val = arr[idx].x[i % 4] + arr[i - 1].y;
        
        /* Use many live variables in computation to keep them alive */
        val = val * v4 + v5 - v6 + v7 * v8 - v9 + v10;
        val = val + v11 - v12 + v13 * v14 - v15 + v16;
        
        /* Complex store addressing - triggers RELOAD_FOR_OUTPUT_ADDRESS */
        /* arr[i].x[0] with additional computation */
        arr[i].x[0] = val * v17 + v18 - v19 + v20;
        
        /* Inline assembly to force address reloads */
        /* This should trigger RELOAD_FOR_OPERAND_ADDRESS */
        asm volatile("# Operand address reload for arr[%0].y" 
                     : 
                     : "m" (arr[idx].y)
                     : "memory");
        
        /* Another inline assembly with more complex addressing */
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address reload for arr[%0].x[%1]" 
                     : 
                     : "m" (arr[(idx * v1 + i) % n].x[(i * v2) % 4])
                     : "memory");
        
        /* Mix in output addressing with inline assembly */
        /* Should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        int temp = arr[i].x[0];
        asm volatile("# Output address reload: %0" 
                     : "=m" (arr[(i * v3 + v4) % n].y)
                     : "r" (temp)
                     : "memory");
        
        sum += arr[i].x[0];
        
        /* Modify variables to prevent optimization */
        v1 += i & 1;
        v2 += i & 2;
        v3 += i & 4;
        v4 += i & 8;
    }
    
    /* Use all variables in final computation */
    return sum + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
           v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 100;
        }
        arr[i].y = (i * 11) % 100;
    }
}

int main(void) {
    /* Use volatile to prevent constant propagation */
    volatile int N = ARRAY_SIZE;
    volatile int stride = 7;
    volatile int offset = 3;
    volatile int scale = 2;
    volatile int mod = 5;
    
    /* Allocate array dynamically to avoid stack overflow */
    struct Data *arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize array with non-constant pattern */
    init_array(arr, N);
    
    /* Perform complex accesses - this is where reloads happen */
    int result = complex_access_loop(arr, N, stride, offset, scale, mod);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional complex access with different pattern */
    stride = 11;
    offset = 5;
    scale = 3;
    mod = 7;
    
    int result2 = complex_access_loop(arr, N / 2, stride, offset, scale, mod);
    printf("Result2: %d\n", result2);
    
    free(arr);
    return 0;
}
