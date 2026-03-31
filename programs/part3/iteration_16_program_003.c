#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define NUM_VARS 25

/* Prevent optimization of helper functions */
__attribute__((noinline, noipa))
void complex_access_loop(int n, volatile int* results);

/* Struct with multiple fields to force complex addressing */
struct Data {
    int x[4];
    int y;
    int z[2];
};

/* Global volatile variables to prevent constant propagation */
volatile int stride1 = 3;
volatile int stride2 = 7;
volatile int offset1 = 11;
volatile int offset2 = 13;
volatile int scale1 = 2;
volatile int scale2 = 5;

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
void complex_access_loop(int n, volatile int* results) {
    struct Data* arr = (struct Data*)malloc(n * sizeof(struct Data));
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            arr[i].x[j] = i * 10 + j;
        }
        arr[i].y = i * 100;
        arr[i].z[0] = i * 1000;
        arr[i].z[1] = i * 10000;
    }
    
    /* Many live variables to consume registers */
    int v1 = stride1;
    int v2 = stride2;
    int v3 = offset1;
    int v4 = offset2;
    int v5 = scale1;
    int v6 = scale2;
    int v7 = n / 2;
    int v8 = n / 3;
    int v9 = n / 4;
    int v10 = n / 5;
    int v11 = 1;
    int v12 = 2;
    int v13 = 3;
    int v14 = 4;
    int v15 = 5;
    int v16 = 6;
    int v17 = 7;
    int v18 = 8;
    int v19 = 9;
    int v20 = 10;
    int v21 = 11;
    int v22 = 12;
    int v23 = 13;
    int v24 = 14;
    int v25 = 15;
    
    int sum = 0;
    
    /* Main loop with complex addressing */
    for (int i = 1; i < n - 1; ++i) {
        /* Complex index calculation using many live variables */
        int idx1 = (i * v1 + v3) / v5;
        int idx2 = (i * v2 + v4) / v6;
        
        /* Ensure indices are in bounds */
        if (idx1 < 0) idx1 = 0;
        if (idx1 >= n) idx1 = n - 1;
        if (idx2 < 0) idx2 = 0;
        if (idx2 >= n) idx2 = n - 1;
        
        /* Complex load with struct field access and array indexing */
        /* This should trigger RELOAD_FOR_INPUT_ADDRESS */
        int val1 = arr[idx1].x[i % 4] + arr[i-1].y;
        
        /* More complex addressing with multiple components */
        /* This should trigger RELOAD_FOR_INPADDR_ADDRESS */
        int val2 = arr[(i * v7 + v8) % n].z[(i * v9) % 2] + 
                   arr[(i * v10 + v11) % n].x[(i * v12) % 4];
        
        /* Complex store - should trigger RELOAD_FOR_OUTPUT_ADDRESS */
        arr[i].x[0] = val1 * v13 + val2 * v14 + v15;
        
        /* Another complex store - should trigger RELOAD_FOR_OUTADDR_ADDRESS */
        arr[(i * v16 + v17) % n].y = val1 * v18 + val2 * v19 + v20;
        
        /* Inline assembly to force address reloads */
        /* Should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OPADDR_ADDR */
        asm volatile("# Complex address 1: %0" : : "m" (arr[idx1].y));
        asm volatile("# Complex address 2: %0" : : "m" (arr[idx2].z[0]));
        
        /* More inline assembly with different addressing */
        /* Should trigger RELOAD_FOR_OTHER_ADDRESS */
        asm volatile("# Other address: %0" : : "m" (arr[(i * v21 + v22) % n].x[(i * v23) % 4]));
        
        /* Use all variables to keep them live */
        sum += val1 + val2 + v24 + v25;
        
        /* Update some variables to prevent optimization */
        v1 = (v1 * 3) % 17;
        v2 = (v2 * 5) % 19;
        v3 = (v3 * 7) % 23;
        v4 = (v4 * 11) % 29;
    }
    
    /* Compute checksum using complex addressing */
    for (int i = 0; i < n; i += 2) {
        int idx = (i * v5 + v6) % n;
        sum += arr[idx].x[0] + arr[idx].y + arr[idx].z[0];
    }
    
    *results = sum;
    
    free(arr);
}

int main() {
    volatile int N = 1024;
    volatile int result = 0;
    
    /* Call the complex function */
    complex_access_loop(N, &result);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional test with smaller array but more complex addressing */
    volatile int N2 = 512;
    volatile int result2 = 0;
    
    complex_access_loop(N2, &result2);
    printf("Result2: %d\n", result2);
    
    return 0;
}
