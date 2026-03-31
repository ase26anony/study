/* Selective scheduling test case targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
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
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* sum = (sum * a[i]) + b[i] pattern */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    /* Uses int, short, char in calculations */
    int threshold = 500;
    short scale = 3;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            result2 += a[i] * scale;
        }
        /* Mix in char type */
        result2 += (int)d[i];
    }
    
    /* Loop 3: Independent chain with bitwise operations */
    /* Creates different dependency pattern */
    for (int i = 0; i < n; i++) {
        result3 = (result3 ^ a[i]) | b[i];
        result3 = result3 & 0xFFFF;  /* Keep it bounded */
    }
    
    /* Loop 4: Short loop with char/short promotion */
    /* Multiple uses of same variable in loop */
    for (int i = 0; i < n; i++) {
        short temp = c[i];
        result4 = result4 + temp;
        result4 = result4 - (temp / 2);  /* Creates use-modify-use chain */
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + (int)result4;
}

int main(void) {
    /* Medium-sized arrays */
    int array_a[SIZE];
    int array_b[SIZE];
    short array_c[SIZE];
    char array_d[SIZE];
    
    /* Initialize with deterministic values */
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function with multiple scheduling opportunities */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination without preventing scheduling */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
