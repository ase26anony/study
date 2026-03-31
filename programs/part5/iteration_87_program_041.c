/* sel-sched-trigger.c - Program to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, char *c, short *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (char)(lcg_rand() % 256);
        d[i] = (short)(lcg_rand() % 10000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int n) {
    int result = 0;
    int temp1, temp2;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    temp1 = 1;
    for (int i = 0; i < n; ++i) {
        /* Chain: use, modify, use pattern */
        temp1 = (temp1 * a[i]) + b[i];
        temp1 = temp1 & 0x7FFFFFFF;  /* Keep within positive range */
    }
    result ^= temp1;
    
    /* Loop 2: Conditional accumulation with type mixing */
    temp2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        /* Mix int and char types with condition */
        if (a[i] > threshold) {
            temp2 += a[i] * (int)c[i % n];
        } else {
            temp2 -= b[i] | (i & 0xFF);
        }
    }
    result += temp2;
    
    /* Loop 3: Short dependency chain with multiple uses */
    int chain = 0;
    for (int i = 1; i < n - 1; ++i) {
        /* Three-operation dependency chain */
        chain = a[i-1] + b[i];
        chain = chain * (int)d[i];
        chain = chain - a[i+1];
        result += chain;
    }
    
    /* Loop 4: Search with early exit possibility */
    int found = 0;
    int target = 750;
    for (int i = 0; i < n && !found; ++i) {
        if (a[i] == target || b[i] == target) {
            found = 1;
            result |= (1 << (i % 16));
        }
        /* Additional computation to give scheduler choices */
        result += (c[i % n] << 2);
    }
    
    /* Loop 5: Reduction with varying stride */
    int sum_even = 0, sum_odd = 0;
    for (int i = 0; i < n; i += 2) {
        sum_even += a[i] * 3;
        if (i + 1 < n) {
            sum_odd += b[i + 1] * 7;
        }
        /* Cross-iteration dependency */
        sum_even = (sum_even ^ sum_odd) & 0xFFFF;
    }
    result += sum_even + sum_odd;
    
    return result;
}

int main(int argc, char **argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 256;
    
    if (n < 10) n = 256;  /* Ensure reasonable size */
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    char *c = (char*)malloc(n * sizeof(char));
    short *d = (short*)malloc(n * sizeof(short));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, d, n);
    
    /* Call work function with multiple scheduling opportunities */
    int result = work(a, b, c, d, n);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = result;
    
    /* Use result to prevent elimination */
    if (sink != 0xDEADBEEF) {  /* Arbitrary check */
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
