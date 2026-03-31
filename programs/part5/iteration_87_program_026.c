/* Selective scheduler trigger program for GCC sel-sched-dump.cc coverage */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with deterministic values */
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
    int result1 = 0, result2 = 0, result3 = 0;
    short result4 = 0;
    int threshold = 500;
    char mask = 0x3F;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < n; ++i) {
        /* Create a dependency chain: use, modify, use */
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 ^ (a[i] & 0xFF);
        result1 = result1 | (b[i] << 3);
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            /* Mix int and short types */
            result2 += a[i] * (int)c[i];
        } else {
            result2 -= b[i] / 2;
        }
        /* Additional operation to create scheduling choices */
        result2 = result2 & 0x7FFFFFFF;
    }
    
    /* Loop 3: Independent short/int operations with carry chain */
    int temp = 0;
    for (int i = 1; i < n - 1; i++) {
        /* Multiple dependent operations */
        temp = a[i-1] + a[i] + a[i+1];
        result3 = result3 ^ temp;
        result3 = result3 * 3 + 1;
        /* Use char type to force promotions */
        result3 += (int)d[i] * 2;
    }
    
    /* Loop 4: Short type loop with bitwise operations */
    for (int i = 0; i < n; i++) {
        /* Create short dependency chain */
        result4 = (result4 << 1) | (c[i] & mask);
        result4 = result4 ^ (c[i] >> 2);
        result4 = result4 + (short)d[i];
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + (int)result4;
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
    
    /* Call work function with multiple scheduling opportunities */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = final_result;
    
    /* Use result in side effect */
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
