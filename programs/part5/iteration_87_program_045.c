/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, short *c, char *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (short)(lcg_rand() % 256);
        d[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int n) {
    int result = 0;
    int temp1, temp2;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    /* Creates tight dependency: result = (result * a[i]) + b[i] */
    int sum1 = 1;
    for (int i = 0; i < n; i++) {
        sum1 = (sum1 * a[i]) + b[i];
    }
    result ^= sum1;
    
    /* Loop 2: Mixed-type operations with condition */
    /* Uses char/short types causing promotions/demotions */
    int sum2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; i++) {
        int val = (int)c[i] * 2;  /* Promotion from short */
        if (val > threshold) {
            sum2 += val * (int)d[i];  /* char promoted to int */
        } else {
            sum2 += val / 2;
        }
    }
    result += sum2;
    
    /* Loop 3: Independent parallel chains */
    /* Two independent dependency chains in same loop */
    int chain_a = 0, chain_b = 0;
    for (int i = 0; i < n; i++) {
        chain_a = (chain_a & a[i]) | b[i];
        chain_b = (chain_b ^ b[i]) + c[i];
    }
    result += chain_a - chain_b;
    
    /* Loop 4: Reduction with varying stride */
    /* Gives scheduler placement choices with different latencies */
    int sum4 = 0;
    for (int i = 0; i < n; i += 2) {
        sum4 += a[i] * 3 + b[i + 1];
    }
    for (int i = 1; i < n; i += 2) {
        sum4 -= b[i] * 2 - a[i - 1];
    }
    result ^= sum4;
    
    /* Loop 5: Search with early exit possibility */
    /* Could generate conditional moves */
    int found = -1;
    int target = 750;
    for (int i = 0; i < n && found < 0; i++) {
        if (a[i] > target && b[i] < target / 2) {
            found = i;
        }
    }
    result += (found * 7);
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination without blocking scheduling */
    volatile int sink = result;
    
    /* Use result to prevent optimization */
    if (sink != 0) {
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
