#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

// Simple deterministic pseudo-random generator
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Initialize arrays with deterministic pseudo-random values
static void init_arrays(int *arr1, int *arr2, char *arr3, short *arr4, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = (char)(lcg_rand() % 256);
        arr4[i] = (short)(lcg_rand() % 1000);
    }
}

// Work function with multiple loops for selective scheduling
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int n) {
    int result = 0;
    int temp1 = 1, temp2 = 0;
    
    // Loop 1: Multiplicative-accumulate with data-dependent chain
    // Creates tight dependency: result -> temp1 -> result
    for (int i = 0; i < n; ++i) {
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;
    }
    
    // Loop 2: Mixed operations with char promotion and condition
    // Creates multiple use-modify-use patterns
    int threshold = 100;
    for (int i = 0; i < n; ++i) {
        int val = (int)c[i];  // Promotion from char to int
        if (val > threshold) {
            temp2 = (temp2 + val) * 3;
        } else {
            temp2 = (temp2 - val) | 0x7F;
        }
        result += temp2;
    }
    
    // Loop 3: Short operations with bitwise mixing
    // Independent dependency chain from previous loops
    int acc = result;
    for (int i = 0; i < n; ++i) {
        short val = d[i];
        acc = (acc & 0xFFFF) + (int)val;  // Demotion then promotion
        acc = (acc << 3) | (acc >> 5);    // Rotation
        result ^= acc;
    }
    
    // Loop 4: Nested dependency with multiple arithmetic ops
    // Gives scheduler complex pattern to analyze
    int chain = 1;
    for (int i = 0; i < n; ++i) {
        chain = chain * 7 + a[i];
        chain = chain - b[i] / 2;
        chain = chain & 0xFFF;
        result += chain;
    }
    
    return result;
}

int main() {
    // Allocate and initialize arrays
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    char *arr3 = (char*)malloc(SIZE * sizeof(char));
    short *arr4 = (short*)malloc(SIZE * sizeof(short));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, SIZE);
    
    // Call work function with array parameters (prevents constant propagation)
    int result = work(arr1, arr2, arr3, arr4, SIZE);
    
    // Use volatile sink to prevent dead code elimination
    volatile int sink = result;
    
    // Simple side effect to ensure code isn't removed
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  // This should never happen
    }
    
    // Print to prevent optimization
    printf("Result: %d\n", result);
    
    // Cleanup
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
