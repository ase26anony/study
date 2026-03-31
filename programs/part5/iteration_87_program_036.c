/* sel-sched-trigger.c
 * Program to trigger selective scheduler debug dumps in GCC
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
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
static void init_arrays(int *a, int *b, char *c, short *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (char)(lcg_rand() % 256);
        d[i] = (short)(lcg_rand() % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* sum = (sum * a[i]) + b[i] pattern */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* Uses char type with promotion */
    int threshold = 100;
    int scale = 3;
    for (int i = 0; i < size; i++) {
        if (c[i] > threshold) {
            result2 += (int)c[i] * scale;
        }
    }
    
    /* Loop 3: Multiple independent operations in same loop */
    /* Creates complex dependency web */
    int temp1 = 0, temp2 = 0;
    for (int i = 0; i < size; i++) {
        temp1 = temp1 ^ a[i];
        temp2 = temp2 | b[i];
        result3 = result3 + (temp1 & temp2);
    }
    
    /* Loop 4: Short dependency chain with different data type */
    /* Uses short with sign extension */
    int accumulator = 0;
    for (int i = 0; i < size; i++) {
        accumulator = accumulator + (int)d[i];
        if (accumulator < 0) {
            accumulator = accumulator * 2;
        }
        result4 = result4 ^ accumulator;
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *array_c = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *array_d = (short*)malloc(ARRAY_SIZE * sizeof(short));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Call work function with arrays */
    int final_result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink __attribute__((unused)) = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (final_result == 0) {
        /* This branch is unlikely but prevents optimization */
        printf("Result is zero\n");
    }
    
    /* Free allocated memory */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
