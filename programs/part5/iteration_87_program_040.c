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
    // Creates a tight dependency chain: result = ((result * a[i]) + b[i])
    int sum1 = 0;
    for (int i = 0; i < n; ++i) {
        sum1 = (sum1 * a[i]) + b[i];
    }
    result ^= sum1;
    
    // Loop 2: Conditional accumulation with type mixing
    // Uses short and char types with promotion
    int sum2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            sum2 += (int)c[i] * (int)d[i];
        }
        // Additional operation to create more scheduling opportunities
        sum2 = (sum2 & 0xFFF) | (b[i] & 0xF00);
    }
    result += sum2;
    
    // Loop 3: Independent computation with bitwise operations
    // Creates parallel dependency chains
    int sum3 = 0;
    int mask = 0x7F;
    for (int i = 0; i < n; i++) {
        int temp = a[i] & mask;
        sum3 = (sum3 << 1) | (temp & 1);
        sum3 ^= b[i];
    }
    result ^= sum3;
    
    // Loop 4: Nested dependency with simple arithmetic
    // Multiple uses of the same variable in different ways
    int sum4 = 1;
    for (int i = 0; i < n; i++) {
        // Create multiple dependency chains
        int x = a[i] + i;
        int y = b[i] - i;
        sum4 = (sum4 * x) / (y != 0 ? y : 1);
        sum4 = sum4 + (c[i] | d[i]);
    }
    result += sum4;
    
    return result;
}

int main() {
    // Allocate and initialize arrays
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    // Call work function with multiple scheduling opportunities
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    // Prevent dead code elimination with volatile sink
    volatile int sink = final_result;
    
    // Use result in a side effect to prevent removal
    if (sink != 0) {
        printf("Result: %d\n", sink);
    }
    
    // Cleanup
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
