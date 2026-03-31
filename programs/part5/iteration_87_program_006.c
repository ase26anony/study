/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debugging output
 * Specifically targets uncovered lines in sel-sched-dump.cc
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, short *c, char *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (short)(lcg_rand() % 256);
        d[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < size; ++i) {
        /* Create a dependency chain: use, modify, use pattern */
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 ^ (a[i] & 0xFF);
    }
    
    /* Loop 2: Conditional operations with type mixing */
    int threshold = 500;
    short scale = 2;
    for (int i = 0; i < size; i++) {
        if (a[i] > threshold) {
            /* Mix int and short types */
            result2 += a[i] * scale;
        }
        /* Simple condition on loop variable */
        if (i % 3 == 0) {
            result2 = result2 | b[i];
        }
    }
    
    /* Loop 3: Independent chain with char/short promotions */
    for (int i = 0; i < size; ++i) {
        /* Create promotion/demotion patterns */
        int temp = (int)c[i] * (int)d[i];
        result3 = result3 + temp;
        result3 = result3 - (d[i] * 2);
    }
    
    /* Loop 4: Multiple uses with simple arithmetic */
    int acc = 0;
    for (int i = 1; i < size - 1; i++) {
        /* Three-operation dependency chain */
        acc = a[i-1] + b[i];
        result4 = result4 ^ (acc * a[i+1]);
        result4 = result4 & 0xFFFF;
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short *array_c = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *array_d = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Call work function with array size to prevent constant propagation */
    int size = ARRAY_SIZE;
    int result = work(array_a, array_b, array_c, array_d, size);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Clean up */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
