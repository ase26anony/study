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
    return seed;
}

/* Initialize arrays with deterministic values */
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
    int result1 = 0, result2 = 0, result3 = 0;
    int result4 = 0;
    char result5 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* sum = (sum * a[i]) + b[i] pattern */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* Uses char type to create promotion instructions */
    int threshold = 50;
    for (int i = 0; i < n; i++) {
        if (c[i] > threshold) {
            result2 += c[i] * 3;  /* char promoted to int */
        } else {
            result2 -= c[i];
        }
    }
    
    /* Loop 3: Multiple independent operations in same loop */
    /* Gives scheduler choices for instruction ordering */
    int temp1 = 0, temp2 = 0;
    for (int i = 0; i < n; i++) {
        temp1 = temp1 ^ a[i];      /* XOR chain */
        temp2 = temp2 | b[i];      /* OR chain */
        result3 = result3 + (temp1 & temp2);  /* Combined dependency */
    }
    
    /* Loop 4: Short dependency chain with short data type */
    /* Creates demotion/promotion patterns */
    short acc = 0;
    for (int i = 0; i < n; i++) {
        acc = (acc + d[i]) * 2;    /* short operations with promotion */
        if (acc > 1000) {
            acc = acc / 3;
        }
        result4 += acc;            /* promoted to int */
    }
    
    /* Loop 5: Nested dependency with bit operations */
    /* Multiple uses of same variable in different ways */
    unsigned int mask = 0xFF;
    for (int i = 0; i < n; i++) {
        unsigned int val = (unsigned int)c[i];
        result5 ^= (char)((val & mask) | ((val >> 2) & 0x3F));
        mask = (mask << 1) | (mask >> 7);  /* rotate mask */
    }
    
    /* Combine all results to prevent elimination */
    int final_result = result1 + result2 + result3 + result4 + (int)result5;
    
    /* Add some final computation to extend live ranges */
    final_result = (final_result * 31) ^ 0x5A5A5A5A;
    
    return final_result;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    char *array_c = (char*)malloc(SIZE * sizeof(char));
    short *array_d = (short*)malloc(SIZE * sizeof(short));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print to prevent elimination (optional) */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
