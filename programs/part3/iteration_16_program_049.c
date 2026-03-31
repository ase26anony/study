#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(int n, volatile int* restrict checksum) {
    /* Force many live variables to consume registers */
    int v1 = 1, v2 = 2, v3 = 3, v4 = 4, v5 = 5, v6 = 6, v7 = 7, v8 = 8;
    int v9 = 9, v10 = 10, v11 = 11, v12 = 12, v13 = 13, v14 = 14, v15 = 15;
    int v16 = 16, v17 = 17, v18 = 18, v19 = 19, v20 = 20;
    int v21 = 21, v22 = 22, v23 = 23, v24 = 24, v25 = 25;
    
    /* Volatile variables to prevent constant propagation */
    volatile int stride = 7;
    volatile int offset = 13;
    volatile int scale = 3;
    volatile int base = 100;
    
    /* Array of structs with nested arrays */
    struct Data {
        int x[4];
        int y;
        int z[2];
    };
    
    /* Dynamically allocate to avoid stack overflow */
    struct Data* arr = (struct Data*)malloc(n * sizeof(struct Data));
    if (!arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = i + j;
        }
        arr[i].y = i * 2;
        arr[i].z[0] = i * 3;
        arr[i].z[1] = i * 4;
    }
    
    int sum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n - 1; ++i) {
        /* Use many variables in complex index calculations */
        int idx1 = (i * v1 + v2 * stride + offset) / scale;
        int idx2 = (i * v3 + v4 * offset - v5) / stride;
        int idx3 = (i * v6 + v7 * scale + v8) / (stride + 1);
        
        /* Ensure indices are within bounds */
        idx1 = idx1 % (n - 2);
        idx2 = idx2 % (n - 2);
        idx3 = idx3 % (n - 2);
        if (idx1 < 0) idx1 = -idx1;
        if (idx2 < 0) idx2 = -idx2;
        if (idx3 < 0) idx3 = -idx3;
        
        /* COMPLEX LOAD: Input address reloads */
        /* Multi-level struct/array access with runtime index */
        int val1 = arr[idx1].x[i % 4] + 
                   arr[idx2].y * v9 + 
                   arr[i-1].z[0] / v10;
        
        /* More complex addressing */
        int val2 = arr[(idx1 * v11 + v12) % n].x[(i + 1) % 4] +
                   arr[(idx2 * v13 - v14) % n].y * v15;
        
        /* COMPLEX STORE: Output address reloads */
        /* Store with complex address calculation */
        arr[i].x[0] = val1 * v16 + val2 * v17 + v18;
        arr[i].x[1] = (val1 + val2) * v19 - v20;
        
        /* Even more complex store addressing */
        int store_idx = (i * v21 + idx3 * v22 + base) % n;
        arr[store_idx].y = val1 * val2 + v23;
        
        /* INLINE ASSEMBLY: Force operand address reloads */
        /* Memory constraint with complex addressing */
        asm volatile("# Complex address 1: %0" 
                     : 
                     : "m" (arr[(idx1 * v24 + v25) % n].z[0])
                     : "memory");
        
        /* Another inline asm with different complex address */
        asm volatile("# Complex address 2: %0" 
                     : 
                     : "m" (arr[store_idx].x[(i % 3) + 1])
                     : "memory");
        
        /* Use all variables to keep them live */
        sum += val1 + val2 + arr[i].x[0] + arr[i].x[1] + 
               arr[store_idx].y + v1 + v2 + v3 + v4 + v5 +
               v6 + v7 + v8 + v9 + v10 + v11 + v12 + v13 +
               v14 + v15 + v16 + v17 + v18 + v19 + v20 +
               v21 + v22 + v23 + v24 + v25;
        
        /* Modify some variables to prevent optimization */
        v1 = (v1 + 1) % 10;
        v2 = (v2 + 2) % 10;
        stride = (stride + 1) % 5 + 1;
        offset = (offset + 2) % 7 + 1;
    }
    
    *checksum = sum;
    
    /* Final complex operation mixing everything */
    for (int i = 0; i < 10 && i < n; i++) {
        int complex_idx = (i * v1 * stride + v2 * offset + v3 * scale) % n;
        asm volatile("# Final complex address: %0"
                     :
                     : "m" (arr[complex_idx].x[i % 4])
                     : "memory");
    }
    
    free(arr);
}

/* Another noinline function to create more reload contexts */
__attribute__((noinline, noipa))
void secondary_complex_access(struct Data* arr, int n, int* result) {
    volatile int mod1 = 17, mod2 = 23, mod3 = 31;
    
    for (int i = 0; i < n; i++) {
        /* Different complex addressing pattern */
        int idx_a = (i * mod1 + mod2) % n;
        int idx_b = (i * mod2 - mod3) % n;
        if (idx_a < 0) idx_a = -idx_a;
        if (idx_b < 0) idx_b = -idx_b;
        
        /* Mixed load/store with complex addresses */
        int temp = arr[idx_a].x[0] + arr[idx_b].y;
        arr[i].z[i % 2] = temp * mod1 + mod2;
        
        /* Inline asm with memory operand */
        asm volatile("# Secondary address: %0"
                     :
                     : "m" (arr[(idx_a + idx_b) % n].x[1])
                     : "memory");
        
        *result += temp;
    }
}

int main() {
    volatile int n = 1024;  /* Volatile to prevent constant propagation */
    int checksum1 = 0, checksum2 = 0;
    
    printf("Starting complex access patterns...\n");
    
    /* First complex access loop */
    complex_access_loop(n, &checksum1);
    
    /* Create another array for secondary function */
    struct Data* arr2 = (struct Data*)malloc(n * sizeof(struct Data));
    if (arr2) {
        for (int i = 0; i < n; i++) {
            arr2[i].x[0] = i * 2;
            arr2[i].x[1] = i * 3;
            arr2[i].x[2] = i * 4;
            arr2[i].x[3] = i * 5;
            arr2[i].y = i * 6;
            arr2[i].z[0] = i * 7;
            arr2[i].z[1] = i * 8;
        }
        
        secondary_complex_access(arr2, n / 2, &checksum2);
        free(arr2);
    }
    
    printf("Checksum 1: %d\n", checksum1);
    printf("Checksum 2: %d\n", checksum2);
    printf("Total: %d\n", checksum1 + checksum2);
    
    return 0;
}
