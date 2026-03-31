/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

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
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight loop with data-dependent chain (multiply-add pattern) */
    /* Creates a dependency chain: result1 = ((result1 * a[i]) + b[i]) */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed operations with short/char promotion */
    /* Uses different data types to create promotion instructions */
    for (int i = 0; i < n; ++i) {
        int temp = (int)c[i] * (int)d[i];
        result2 = result2 ^ temp;  /* XOR breaks simple dependency pattern */
        result2 = result2 + (temp & 0xFF);
    }
    
    /* Loop 3: Conditional loop with simple condition */
    /* May generate conditional moves for speculative execution */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            result3 += a[i] * 3;
        } else {
            result3 -= b[i];
        }
    }
    
    /* Loop 4: Independent parallel-like computation */
    /* Gives scheduler multiple independent chains to work with */
    int acc1 = 0, acc2 = 0;
    for (int i = 0; i < n; ++i) {
        acc1 = acc1 | a[i];      /* OR operation */
        acc2 = acc2 & b[i];      /* AND operation */
        result4 = result4 + (acc1 ^ acc2);
    }
    
    /* Combine all results to prevent elimination */
    return result1 + result2 + result3 + result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink __attribute__((unused)) = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (final_result == 0) {
        /* This branch is unlikely but prevents optimization */
        __builtin_unreachable();
    }
    
    /* Clean up */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    /* Print something to ensure execution */
    printf("Result: %d\n", final_result);
    
    return 0;
}
