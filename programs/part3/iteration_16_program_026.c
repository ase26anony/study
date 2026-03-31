#include <stdio.h>
#include <stdlib.h>

#define NUM_VARS 20

struct Data {
    int x[4];
    int y;
};

// Force complex addressing by preventing optimization
volatile int stride = 3;
volatile int offset = 7;
volatile int scale = 2;
volatile int extra1 = 11;
volatile int extra2 = 13;

// Prevent inlining to maintain code shape
__attribute__((noinline, noipa))
int complex_access_loop(struct Data *arr, int n) {
    // Declare many local variables to exhaust registers
    int v1 = stride;
    int v2 = offset;
    int v3 = scale;
    int v4 = extra1;
    int v5 = extra2;
    int v6 = n * 2;
    int v7 = n / 3;
    int v8 = n + 5;
    int v9 = n - 2;
    int v10 = n * 3;
    int v11 = n / 2;
    int v12 = n + 7;
    int v13 = n - 3;
    int v14 = n * 4;
    int v15 = n / 4;
    int v16 = n + 9;
    int v17 = n - 4;
    int v18 = n * 5;
    int v19 = n / 5;
    int v20 = n + 11;
    
    int sum = 0;
    
    // Complex loop with multiple addressing modes
    for (int i = 1; i < n; ++i) {
        // Use all variables to keep them live
        int temp1 = v1 + v2 + v3 + v4 + v5;
        int temp2 = v6 + v7 + v8 + v9 + v10;
        int temp3 = v11 + v12 + v13 + v14 + v15;
        int temp4 = v16 + v17 + v18 + v19 + v20;
        
        // Complex index calculation that cannot be encoded in one instruction
        // This should trigger RELOAD_FOR_INPUT_ADDRESS
        int idx = (i * v1 + v2) / v3;
        idx = (idx * v4 + v5) % n;
        if (idx < 0) idx = -idx;
        
        // Complex addressing for load - should trigger various input address reloads
        int val = arr[idx].x[i % 4] + arr[i-1].y;
        val += temp1 - temp2 + temp3 - temp4;
        
        // More complex index for store - should trigger output address reloads
        int idx2 = (i * v6 + v7) / v8;
        idx2 = (idx2 * v9 + v10) % n;
        if (idx2 < 0) idx2 = -idx2;
        
        // Complex addressing for store - should trigger RELOAD_FOR_OUTPUT_ADDRESS
        arr[idx2].x[0] = val * v11 + v12;
        
        // Even more complex addressing with struct field access
        int idx3 = (i * v13 + v14) / v15;
        idx3 = (idx3 * v16 + v17) % n;
        if (idx3 < 0) idx3 = -idx3;
        
        // Store with different complex address
        arr[i].x[1] = arr[idx3].x[2] * v18 + v19;
        
        // Inline assembly to force specific reload types
        // This should trigger RELOAD_FOR_OPERAND_ADDRESS and RELOAD_FOR_OTHER_ADDRESS
        asm volatile("# Complex address operand %0" 
                     : 
                     : "m" (arr[(i * v20 + v1) % n].y));
        
        // Another inline assembly with different addressing
        asm volatile("# Another address %0" 
                     : 
                     : "m" (arr[idx].x[(i + v2) % 4]));
        
        // Mix in some pointer arithmetic that needs reloads
        struct Data *ptr1 = &arr[(i * v3 + v4) % n];
        struct Data *ptr2 = &arr[(i * v5 + v6) % n];
        
        // Access through pointers with offset
        ptr1->x[2] = ptr2->x[3] + v7;
        
        // Update sum to prevent dead code elimination
        sum += arr[i].x[0] + arr[i].x[1] + arr[idx].y;
        
        // Modify some variables to prevent optimization
        v1 = (v1 + 1) % 17;
        v2 = (v2 + 2) % 19;
        v3 = (v3 + 3) % 23;
    }
    
    return sum;
}

// Another noinline function to create more reload contexts
__attribute__((noinline, noipa))
void init_array(struct Data *arr, int n) {
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < 4; ++j) {
            arr[i].x[j] = (i * 7 + j * 13) % 97;
        }
        arr[i].y = (i * 11) % 89;
    }
}

int main() {
    volatile int N = 1024;  // volatile to prevent constant propagation
    int size = N;
    
    // Allocate array dynamically to avoid stack overflow
    struct Data *arr = (struct Data*)malloc(size * sizeof(struct Data));
    if (!arr) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize array
    init_array(arr, size);
    
    // Perform complex accesses
    int result = complex_access_loop(arr, size);
    
    // Compute checksum
    int checksum = 0;
    for (int i = 0; i < size; ++i) {
        checksum += arr[i].x[0] + arr[i].x[1] + arr[i].x[2] + arr[i].x[3] + arr[i].y;
        checksum %= 1000000;  // Prevent overflow
    }
    
    // Use result and checksum to prevent optimization
    printf("Result: %d, Checksum: %d\n", result, checksum);
    
    free(arr);
    return 0;
}
