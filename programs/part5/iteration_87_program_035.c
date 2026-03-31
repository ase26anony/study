/* sel-sched-test.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(const int* restrict a, const int* restrict b, 
                const short* restrict c, const char* restrict d,
                int n, int m) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int prod = 1;
    short min_val = 32767;
    char xor_result = 0;
    
    /* Loop 1: Mixed operations with loop-carried dependency */
    for (int i = 0; i < n; ++i) {
        /* Chain: use -> modify -> use pattern */
        sum1 = (sum1 * a[i]) + b[i];
        /* Independent operation in same iteration */
        prod = prod & (a[i] | 1);
    }
    
    /* Loop 2: Conditional with mixed data types */
    for (int i = 0; i < m; ++i) {
        /* Promote char to int for arithmetic */
        int val = d[i] * 2;
        if (val > 100) {
            /* Multiple uses of same computed value */
            sum2 += val * c[i];
            sum2 = sum2 | (val & 0xFF);
        }
        /* Always execute path */
        min_val = (c[i] < min_val) ? c[i] : min_val;
    }
    
    /* Loop 3: Short dependency chains with branching */
    for (int i = 1; i < n - 1; ++i) {
        /* Three-operation chain */
        int diff = a[i] - b[i];
        int scaled = diff * 3;
        sum3 += scaled / 2;
        
        /* Another independent chain */
        xor_result ^= d[i % m];
    }
    
    /* Loop 4: Search with early exit possibility */
    int found = 0;
    for (int i = 0; i < n && !found; ++i) {
        if (a[i] == b[i] && a[i] > 1000) {
            found = 1;
            sum3 *= 2;
        }
        /* Always executed operation */
        xor_result += i & 0x7F;
    }
    
    /* Combine all results */
    return sum1 + sum2 + sum3 + prod + min_val + xor_result;
}

int main(void) {
    const int N = 256;
    const int M = 128;
    
    /* Initialize arrays with deterministic values */
    int* array_a = (int*)malloc(N * sizeof(int));
    int* array_b = (int*)malloc(N * sizeof(int));
    short* array_c = (short*)malloc(M * sizeof(short));
    char* array_d = (char*)malloc(M * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    /* Fill arrays with pseudo-random but deterministic values */
    for (int i = 0; i < N; ++i) {
        array_a[i] = (int)(lcg_rand() % 2000) - 1000;
        array_b[i] = (int)(lcg_rand() % 2000) - 1000;
    }
    
    for (int i = 0; i < M; ++i) {
        array_c[i] = (short)(lcg_rand() % 1000);
        array_d[i] = (char)(lcg_rand() % 256);
    }
    
    /* Call work function with multiple scheduling opportunities */
    int result = work(array_a, array_b, array_c, array_d, N, M);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink != result) {
        __builtin_trap();
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
