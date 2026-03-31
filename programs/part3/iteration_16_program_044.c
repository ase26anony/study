#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static void complex_access_loop(volatile int n, volatile int stride, 
                               volatile int offset, volatile int scale) {
    /* Large struct array to force complex addressing */
    struct Data {
        int x[4];
        int y;
        int z[2];
    };
    
    struct Data *arr = malloc(n * sizeof(struct Data));
    if (!arr) return;
    
    /* Initialize with some data */
    for (int i = 0; i < n; i++) {
        arr[i].x[0] = i;
        arr[i].x[1] = i * 2;
        arr[i].x[2] = i * 3;
        arr[i].x[3] = i * 4;
        arr[i].y = i * 5;
        arr[i].z[0] = i * 6;
        arr[i].z[1] = i * 7;
    }
    
    /* Many live variables to exhaust registers */
    int v1 = stride + 1;
    int v2 = offset + 2;
    int v3 = scale + 3;
    int v4 = stride * 2;
    int v5 = offset * 2;
    int v6 = scale * 2;
    int v7 = v1 + v2;
    int v8 = v2 + v3;
    int v9 = v3 + v4;
    int v10 = v4 + v5;
    int v11 = v5 + v6;
    int v12 = v6 + v7;
    int v13 = v7 + v8;
    int v14 = v8 + v9;
    int v15 = v9 + v10;
    int v16 = v10 + v11;
    int v17 = v11 + v12;
    int v18 = v12 + v13;
    int v19 = v13 + v14;
    int v20 = v14 + v15;
    int v21 = v15 + v16;
    int v22 = v16 + v17;
    int v23 = v17 + v18;
    int v24 = v18 + v19;
    int v25 = v19 + v20;
    
    /* Complex loop with addressing that requires multiple reloads */
    for (int i = 1; i < n - 1; ++i) {
        /* Complex index calculation using many live variables */
        int idx = (i * v1 + v2) / v3;
        idx = (idx + v4) * v5 - v6;
        idx = (idx % (n - 2)) + 1;
        
        /* Ensure idx stays in bounds */
        if (idx < 0) idx = 0;
        if (idx >= n) idx = n - 1;
        
        /* Complex load with struct field access - triggers INPUT address reloads */
        int val1 = arr[idx].x[i % 4] + arr[i-1].y * v7;
        int val2 = arr[i].z[0] * v8 + arr[idx].z[1] * v9;
        
        /* More complex calculations keeping variables live */
        int tmp1 = val1 * v10 + val2 * v11;
        int tmp2 = val1 * v12 - val2 * v13;
        int tmp3 = tmp1 * v14 + tmp2 * v15;
        int tmp4 = tmp1 * v16 - tmp2 * v17;
        
        /* Complex store - triggers OUTPUT address reloads */
        arr[i].x[0] = tmp3 * v18 + v19;
        arr[i].x[1] = tmp4 * v20 - v21;
        
        /* Different complex store with another calculation */
        int idx2 = (i * v22 + v23) / v24;
        idx2 = (idx2 % (n - 3)) + 1;
        if (idx2 < 0) idx2 = 0;
        if (idx2 >= n) idx2 = n - 1;
        
        arr[idx2].y = tmp3 * v25 + tmp4 * v1;
        
        /* Inline assembly to force OPERAND address reloads */
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v2 + v3) / v4].x[(i + v5) % 4]));
        
        /* Another inline assembly with different addressing */
        asm volatile("# Output address %0" 
                     : "=m" (arr[(idx * v6 + v7) % n].z[i % 2]));
        
        /* Mix all variables to keep them live */
        v1 = v1 + v2;
        v2 = v2 + v3;
        v3 = v3 + v4;
        v4 = v4 + v5;
        v5 = v5 + v6;
        v6 = v6 + v7;
        v7 = v7 + v8;
        v8 = v8 + v9;
        v9 = v9 + v10;
        v10 = v10 + v11;
        v11 = v11 + v12;
        v12 = v12 + v13;
        v13 = v13 + v14;
        v14 = v14 + v15;
        v15 = v15 + v16;
        v16 = v16 + v17;
        v17 = v17 + v18;
        v18 = v18 + v19;
        v19 = v19 + v20;
        v20 = v20 + v21;
        v21 = v21 + v22;
        v22 = v22 + v23;
        v23 = v23 + v24;
        v24 = v24 + v25;
        v25 = v25 + i;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].y;
    }
    
    printf("Checksum: %d\n", checksum);
    
    free(arr);
}

/* Another noinline function with different pattern */
__attribute__((noinline, noipa))
static void trigger_more_reloads(volatile int n) {
    /* Array of pointers to force indirection */
    int **ptr_arr = malloc(n * sizeof(int*));
    int *data = malloc(n * n * sizeof(int));
    
    if (!ptr_arr || !data) {
        free(ptr_arr);
        free(data);
        return;
    }
    
    /* Setup pointer array with complex strides */
    for (int i = 0; i < n; i++) {
        ptr_arr[i] = &data[i * n];
        for (int j = 0; j < n; j++) {
            ptr_arr[i][j] = i * j;
        }
    }
    
    /* Many live variables again */
    int a1 = n + 1, a2 = n + 2, a3 = n + 3, a4 = n + 4, a5 = n + 5;
    int a6 = n + 6, a7 = n + 7, a8 = n + 8, a9 = n + 9, a10 = n + 10;
    int a11 = n + 11, a12 = n + 12, a13 = n + 13, a14 = n + 14, a15 = n + 15;
    
    /* Loop with pointer chasing and complex addressing */
    for (int i = 1; i < n - 1; i++) {
        /* Complex pointer arithmetic */
        int *ptr1 = ptr_arr[(i * a1 + a2) % n];
        int *ptr2 = ptr_arr[(i * a3 + a4) % n];
        
        /* Load through complex pointer addressing */
        int val1 = ptr1[(i * a5 + a6) % n];
        int val2 = ptr2[(i * a7 + a8) % n];
        
        /* Store through complex addressing */
        ptr_arr[i][(i * a9 + a10) % n] = val1 * a11 + val2 * a12;
        
        /* More complex addressing with multiple indices */
        int idx1 = (i * a13 + a14) % n;
        int idx2 = (i * a15 + a1) % n;
        ptr_arr[idx1][idx2] = val1 * a2 + val2 * a3;
        
        /* Inline assembly with memory constraint */
        asm volatile("# Memory operand %0 %1" 
                     : 
                     : "m" (ptr_arr[(i * a4 + a5) % n][(i * a6 + a7) % n]),
                       "m" (ptr_arr[idx1][(idx2 * a8) % n]));
        
        /* Update variables to keep them live */
        a1 += a2; a2 += a3; a3 += a4; a4 += a5; a5 += a6;
        a6 += a7; a7 += a8; a8 += a9; a9 += a10; a10 += a11;
        a11 += a12; a12 += a13; a13 += a14; a14 += a15; a15 += i;
    }
    
    free(ptr_arr);
    free(data);
}

int main(void) {
    /* Volatile to prevent constant propagation */
    volatile int N = 512;
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    
    /* Call functions that trigger complex reloads */
    complex_access_loop(N, stride, offset, scale);
    trigger_more_reloads(N / 2);
    
    return 0;
}
