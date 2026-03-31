/* sel-sched-test.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(const int* data_a, const int* data_b, 
                const char* flags, int n) {
    int result1 = 0;
    int result2 = 0;
    short result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Mixed operations with loop-carried dependency */
    for (int i = 0; i < n; ++i) {
        /* Complex dependency chain: use, modify, use */
        result1 = (result1 * data_a[i]) + data_b[i];
        result1 = result1 ^ (data_a[i] & 0xFF);
    }
    
    /* Loop 2: Conditional accumulation with mixed types */
    for (int i = 0; i < n; i += 2) {
        char threshold = 50;
        if (data_a[i] > threshold) {
            /* Mix of int and char operations */
            result2 += (data_a[i] * 3) + (flags[i % 256] & 0x7F);
        } else {
            result2 -= data_b[i] / 2;
        }
    }
    
    /* Loop 3: Short type operations with bitwise dependencies */
    for (int i = 1; i < n - 1; ++i) {
        /* Multiple uses of same value with different operations */
        short temp = (short)(data_a[i] + data_b[i-1]);
        result3 = (result3 | temp) & (short)(data_b[i] ^ 0x55);
        result3 = result3 + (short)(data_a[i] * 2);
    }
    
    /* Loop 4: Nested dependency pattern */
    int acc = 100;
    for (int i = 0; i < n; ++i) {
        /* Chain of dependent operations */
        int val = data_a[i] + acc;
        val = val * (data_b[i] % 32);
        val = val | (flags[i % 256] * 2);
        result4 = result4 ^ val;
        acc = val & 0xFFF;  /* Loop-carried dependency */
    }
    
    /* Combine all results */
    return result1 + result2 + (int)result3 + result4;
}

int main(void) {
    const int N = 256;
    int* array_a = (int*)malloc(N * sizeof(int));
    int* array_b = (int*)malloc(N * sizeof(int));
    char* flags = (char*)malloc(256 * sizeof(char));
    
    /* Initialize with deterministic pseudo-random values */
    for (int i = 0; i < N; ++i) {
        array_a[i] = (int)(lcg_rand() % 100);
        array_b[i] = (int)(lcg_rand() % 100);
    }
    for (int i = 0; i < 256; ++i) {
        flags[i] = (char)(lcg_rand() % 128);
    }
    
    /* Call work function multiple times to increase scheduling opportunities */
    int total = 0;
    for (int iter = 0; iter < 3; ++iter) {
        total += work(array_a, array_b, flags, N - iter);
    }
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = total;
    
    /* Use result to prevent optimization */
    if (sink != 0) {
        printf("Result: %d\n", sink);
    }
    
    free(array_a);
    free(array_b);
    free(flags);
    
    return 0;
}
