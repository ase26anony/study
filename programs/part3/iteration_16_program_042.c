#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
int complex_access_loop(volatile int n, volatile int stride, 
                       volatile int offset, volatile int scale);

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Global array to prevent constant propagation */
struct Data *arr;

/* Many live variables to consume registers */
__attribute__((noinline, noipa))
int complex_access_loop(volatile int n, volatile int stride, 
                       volatile int offset, volatile int scale) {
    /* Declare many local variables to exhaust registers */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25;
    
    /* Initialize variables with non-constant values */
    v1 = stride + 1;
    v2 = offset + 2;
    v3 = scale + 3;
    v4 = n + 4;
    v5 = stride * 2;
    v6 = offset * 3;
    v7 = scale * 4;
    v8 = n * 5;
    v9 = stride + offset;
    v10 = scale + n;
    v11 = stride * offset;
    v12 = scale * n;
    v13 = v1 + v2;
    v14 = v3 + v4;
    v15 = v5 + v6;
    v16 = v7 + v8;
    v17 = v9 + v10;
    v18 = v11 + v12;
    v19 = v13 + v14;
    v20 = v15 + v16;
    v21 = v17 + v18;
    v22 = v19 + v20;
    v23 = v21 + v22;
    v24 = v23 * 2;
    v25 = v24 / 3;
    
    int sum = 0;
    
    /* Loop with complex addressing that forces reloads */
    for (int i = 1; i < n; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2 * v3 - v4 / v5 + v6 % v7) / v8;
        
        /* Ensure idx stays within bounds */
        if (idx < 0) idx = -idx;
        idx = idx % (n - 1);
        
        /* Complex load with struct field access - forces INPUT address reloads */
        int val1 = arr[idx].x[i % 4] + arr[i - 1].y * v9;
        int val2 = arr[(i * v10 + v11) / v12].z[i % 2] + v13;
        
        /* More complex calculations keeping variables live */
        int val3 = val1 * v14 + val2 * v15 - v16;
        int val4 = (val3 + v17) * v18 / v19 + v20;
        
        /* Complex store - forces OUTPUT address reloads */
        arr[i].x[(i + v21) % 4] = val4 + v22;
        arr[(i * v23 + v24) / v25].y = val3 - v1;
        
        /* Even more complex addressing for another store */
        int idx2 = (i * v2 + v3 * v4 - v5 / v6 + v7 % v8) / v9;
        if (idx2 < 0) idx2 = -idx2;
        idx2 = idx2 % (n - 1);
        
        arr[idx2].z[(i + v10) % 2] = val1 + val2 + v11;
        
        /* Inline assembly to force OPERAND address reloads */
        /* Input address reload */
        asm volatile("# Input address: %0" : : "m" (arr[idx].x[0]));
        
        /* Output address reload */
        asm volatile("# Output address: %0" : : "m" (arr[i].y));
        
        /* Complex address as operand */
        asm volatile("# Complex operand: %0" : : "m" (arr[(i * v12 + v13) / v14].z[0]));
        
        /* Keep sum to prevent dead code elimination */
        sum += arr[i].x[0] + arr[i].y + arr[i].z[0];
        
        /* Update some variables to prevent optimization */
        v1 += i % 3;
        v2 += i % 5;
        v3 += i % 7;
        v4 += i % 11;
    }
    
    return sum;
}

/* Another function to force different reload contexts */
__attribute__((noinline, noipa))
void mixed_address_operations(volatile int n) {
    /* Different addressing patterns */
    for (int i = 0; i < n; ++i) {
        /* Nested struct/array addressing */
        int idx1 = (i * 7 + 3) / 2;
        int idx2 = (i * 11 + 5) / 3;
        
        /* Complex addressing in both source and destination */
        arr[idx1].x[(i + idx2) % 4] = 
            arr[idx2].y * arr[i].z[i % 2] + 
            arr[(i * 13 + 7) / 4].x[(i + 1) % 4];
        
        /* More inline assembly with different constraints */
        asm volatile("# Mixed address 1: %0" : : "m" (arr[idx1].y));
        asm volatile("# Mixed address 2: %0" : : "m" (arr[idx2].z[0]));
        
        /* Force address computation for pointer arithmetic */
        struct Data *ptr1 = &arr[(i * 17 + 9) / 5];
        struct Data *ptr2 = &arr[(i * 19 + 11) / 6];
        
        ptr1->x[0] = ptr2->y + i;
    }
}

int main(void) {
    volatile int N = 1024;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Allocate and initialize array */
    arr = (struct Data*)malloc(N * sizeof(struct Data));
    if (!arr) return 1;
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 3 + j * 7) % 100;
        }
        arr[i].y = (i * 5 + 11) % 100;
        arr[i].z[0] = (i * 7 + 13) % 100;
        arr[i].z[1] = (i * 11 + 17) % 100;
    }
    
    /* Call functions that trigger different reload types */
    int sum1 = complex_access_loop(N, stride, offset, scale);
    mixed_address_operations(N / 2);
    
    /* Compute final checksum */
    int final_sum = 0;
    for (int i = 0; i < N; ++i) {
        final_sum += arr[i].x[0] + arr[i].y + arr[i].z[0];
    }
    
    final_sum += sum1;
    
    printf("Checksum: %d\n", final_sum);
    
    free(arr);
    return 0;
}
