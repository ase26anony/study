/* sel-sched-trigger.c - Program to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int simple_rand(unsigned int seed) {
    return seed * 1103515245u + 12345u;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int* a, int* b, char* c, short* d, int size, unsigned int seed) {
    unsigned int r = seed;
    for (int i = 0; i < size; i++) {
        r = simple_rand(r);
        a[i] = (r % 100) - 50;  /* Values between -50 and 49 */
        
        r = simple_rand(r);
        b[i] = (r % 200) - 100; /* Values between -100 and 99 */
        
        r = simple_rand(r);
        c[i] = (char)(r % 256); /* Full byte range */
        
        r = simple_rand(r);
        d[i] = (short)(r % 65536); /* Full short range */
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int* a, int* b, char* c, short* d, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight loop with data-dependent chain (multiplication and addition) */
    /* Creates a dependency chain: result1 is used and modified each iteration */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed data types with promotion/demotion */
    /* char -> int promotion, short -> int promotion, different operations */
    for (int i = 0; i < size; ++i) {
        int temp = (int)c[i] * 3;      /* char promoted to int */
        temp += (int)d[i] / 2;         /* short promoted to int */
        result2 += temp & 0xFF;        /* Masking operation */
    }
    
    /* Loop 3: Conditional loop with simple condition */
    /* Creates opportunities for conditional moves/speculation */
    int threshold = 25;
    for (int i = 0; i < size; ++i) {
        if (a[i] > threshold) {
            result3 += a[i] * 2;
        } else {
            result3 -= b[i];
        }
    }
    
    /* Loop 4: Another dependency chain with bitwise operations */
    /* Different dependency pattern from loop 1 */
    for (int i = 0; i < size; ++i) {
        result4 = (result4 ^ a[i]) | b[i];
        result4 = result4 + (c[i] & 0xF);
    }
    
    /* Combine all results to prevent elimination of any loop */
    return result1 + result2 + result3 + result4;
}

int main(int argc, char** argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int size = 256;
    if (argc > 1) {
        size = atoi(argv[1]);
        if (size < 100) size = 100;
        if (size > 500) size = 500;
    }
    
    /* Allocate and initialize arrays */
    int* a = (int*)malloc(size * sizeof(int));
    int* b = (int*)malloc(size * sizeof(int));
    char* c = (char*)malloc(size * sizeof(char));
    short* d = (short*)malloc(size * sizeof(short));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(a, b, c, d, size, 42u);
    
    /* Call work function with multiple loops */
    int result = work(a, b, c, d, size);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
