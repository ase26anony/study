#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

// Simple deterministic pseudo-random generator
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

// Initialize arrays with deterministic values
static void init_arrays(int *a, int *b, short *c, char *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (short)(lcg_rand() % 256);
        d[i] = (char)(lcg_rand() % 128);
    }
}

// Work function with multiple loops for selective scheduling
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int n) {
    int result = 0;
    
    // Loop 1: Data-dependent chain with mixed operations
    // Creates a tight dependency chain: result = (result * a[i]) + b[i]
    int sum1 = 1;
    for (int i = 0; i < n; ++i) {
        sum1 = (sum1 * a[i]) + b[i];
    }
    result ^= sum1;
    
    // Loop 2: Conditional accumulation with type mixing
    // int + char * short operations with condition
    int sum2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            sum2 += (int)d[i] * (int)c[i % (n/2)];
        }
    }
    result ^= sum2;
    
    // Loop 3: Reduction with bitwise operations and loop-carried dependency
    // Creates: acc = (acc & mask) | (value ^ acc)
    unsigned int acc = 0x5A5A5A5A;
    for (int i = 0; i < n; i++) {
        unsigned int val = (unsigned int)b[i];
        acc = (acc & 0x00FFFFFF) | (val ^ acc);
    }
    result ^= (int)acc;
    
    // Loop 4: Search loop with early exit possibility
    // Multiple uses of loop variable with arithmetic
    int found = -1;
    int target = 750;
    for (int i = 0; i < n; i++) {
        int test = a[i] + (b[i] >> 2);
        if (test > target && found == -1) {
            found = i;
        }
    }
    result ^= found;
    
    // Loop 5: Nested dependency with multiple operations
    // Complex enough for scheduler to consider reordering
    int sum3 = 0;
    for (int i = 1; i < n - 1; i++) {
        int left = a[i - 1];
        int center = b[i];
        int right = a[i + 1];
        sum3 += (left * center) | (center & right);
    }
    result ^= sum3;
    
    return result;
}

int main() {
    // Allocate and initialize arrays
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    // Call work function with array size to prevent constant propagation
    int size = SIZE;
    int result = work(array_a, array_b, array_c, array_d, size);
    
    // Use volatile sink to prevent dead code elimination
    volatile int sink = result;
    
    // Simple side effect to ensure code isn't removed
    if (sink == 0x12345678) {  // Unlikely value
        __builtin_trap();
    }
    
    // Cleanup
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
