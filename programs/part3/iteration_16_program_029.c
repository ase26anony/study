#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
static int complex_access_loop(int n, volatile int* params);

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z;
    int w[2];
};

/* Global volatile variables to prevent constant propagation */
volatile int g_stride = 3;
volatile int g_offset = 7;
volatile int g_scale = 2;
volatile int g_mod = 5;

int main(void) {
    const int N = 1024;
    struct Data arr[N];
    
    /* Initialize array with deterministic pattern */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23) % 100;
        arr[i].z = (i * 29) % 100;
        arr[i].w[0] = (i * 31) % 100;
        arr[i].w[1] = (i * 37) % 100;
    }
    
    /* Parameters for complex addressing */
    volatile int params[8];
    for (int i = 0; i < 8; i++) {
        params[i] = (i * 19 + 11) % 20 + 1;
    }
    
    /* Call the complex access function */
    int checksum = complex_access_loop(N, params);
    
    /* Use checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}

__attribute__((noinline, noipa))
static int complex_access_loop(int n, volatile int* params) {
    struct Data* arr = malloc(n * sizeof(struct Data));
    if (!arr) return -1;
    
    /* Initialize local array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = (i * 17 + j * 13) % 100;
        }
        arr[i].y = (i * 23) % 100;
        arr[i].z = (i * 29) % 100;
        arr[i].w[0] = (i * 31) % 100;
        arr[i].w[1] = (i * 37) % 100;
    }
    
    /* Many live variables to consume registers */
    int v1 = params[0];
    int v2 = params[1];
    int v3 = params[2];
    int v4 = params[3];
    int v5 = params[4];
    int v6 = params[5];
    int v7 = params[6];
    int v8 = params[7];
    int v9 = v1 + v2;
    int v10 = v3 * v4;
    int v11 = v5 - v6;
    int v12 = v7 / (v8 ? v8 : 1);
    int v13 = v9 ^ v10;
    int v14 = v11 | v12;
    int v15 = v13 & v14;
    int v16 = v1 * v3 + v5;
    int v17 = v2 * v4 + v6;
    int v18 = v7 * v8 - v9;
    int v19 = v10 / (v11 ? v11 : 1) + v12;
    int v20 = v13 - v14 + v15;
    int v21 = v16 * v17;
    int v22 = v18 + v19;
    int v23 = v20 * v21;
    int v24 = v22 - v23;
    int v25 = v24 % (v25 ? v25 : 1);
    
    int checksum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n - 1; ++i) {
        /* Complex index calculation using many live variables */
        int idx = ((i * v1 + v2) * v3 + v4) / (v5 ? v5 : 1);
        idx = (idx + v6 * v7 - v8) % (n - 2);
        if (idx < 0) idx = -idx;
        
        /* Even more complex index for output */
        int idx_out = ((i * v9 + v10) * v11 + v12) / (v13 ? v13 : 1);
        idx_out = (idx_out + v14 * v15 - v16) % (n - 2);
        if (idx_out < 0) idx_out = -idx_out;
        
        /* Complex addressing for input (LOAD) - triggers input address reloads */
        int val1 = arr[idx].x[i % 4] + arr[i-1].y;
        int val2 = arr[idx + 1].z * arr[i].w[i % 2];
        int val3 = arr[(idx * v17 + v18) % n].x[(i + 1) % 4];
        
        /* Mix in all live variables to keep them active */
        val1 = val1 * v19 + v20;
        val2 = val2 / (v21 ? v21 : 1) - v22;
        val3 = val3 + v23 * v24 - v25;
        
        /* Complex addressing for output (STORE) - triggers output address reloads */
        arr[idx_out].x[0] = val1 + val2;
        arr[idx_out].y = val2 * v1 + v3;
        arr[(idx_out * v4 + v5) % n].z = val3 - v6;
        
        /* Even more complex store addressing */
        int idx_store = ((i * v7 + v8) * v9 + v10) % n;
        arr[idx_store].w[0] = val1 * v11 + val2 * v12;
        arr[(idx_store + v13) % n].w[1] = val3 * v14 - v15;
        
        /* Inline assembly to force operand address reloads */
        asm volatile("# Input address operand %0" : : "m" (arr[idx].y));
        asm volatile("# Output address operand %0" : : "m" (arr[idx_out].x[0]));
        
        /* More complex inline assembly with computed addresses */
        int* addr1 = &arr[idx].x[i % 4];
        int* addr2 = &arr[idx_out].y;
        asm volatile("# Memory operand 1 %0" : : "m" (*addr1));
        asm volatile("# Memory operand 2 %0" : : "m" (*addr2));
        
        /* Update checksum with complex addressing */
        checksum += arr[i].x[0] + arr[i].y + arr[i].z;
        checksum += arr[idx].w[0] + arr[idx_out].w[1];
        
        /* Update live variables to create dependencies */
        v1 = (v1 + i) % 100;
        v2 = (v2 * i + 1) % 100;
        v3 = (v3 ^ i) % 100;
        v4 = (v4 - i) % 100;
        v5 = (v5 + checksum) % 100;
        v6 = (v6 * 3 + i) % 100;
        v7 = (v7 / (i ? i : 1) + 1) % 100;
        v8 = (v8 ^ checksum) % 100;
    }
    
    /* Final complex computation using all variables */
    checksum += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    checksum += v9 + v10 + v11 + v12 + v13 + v14 + v15;
    checksum += v16 + v17 + v18 + v19 + v20 + v21 + v22;
    checksum += v23 + v24 + v25;
    
    free(arr);
    return checksum;
}
