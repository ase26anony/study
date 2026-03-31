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
    int result1 = 0, result2 = 0, result3 = 0;
    char result4 = 0;
    short result5 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* Creates a long dependency chain the scheduler might try to break */
    for (int i = 0; i < n; ++i) {
        /* Multiple uses of result1 with different operations */
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 ^ (a[i] & 0xFF);
        result1 = result1 | (b[i] << 3);
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* Gives scheduler opportunities for speculative execution */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            result2 += a[i] * 3;
        } else {
            result2 -= b[i] / 2;
        }
        /* Additional operation to create more scheduling choices */
        result2 = result2 & 0x7FFFFFFF;  /* Keep positive */
    }
    
    /* Loop 3: Mixed data types with promotion/demotion */
    /* Creates various instruction types for scheduling */
    for (int i = 0; i < n; ++i) {
        /* Mix char, short, int operations */
        int temp = (int)c[i] * 2;
        temp += (int)d[i] / 4;
        result3 = (result3 + temp) * 13;
        /* Modulo to prevent overflow from dominating scheduling */
        result3 = result3 % 1000000;
    }
    
    /* Loop 4: Character processing with data-dependent chain */
    /* Different dependency pattern from previous loops */
    for (int i = 0; i < n; ++i) {
        result4 = (result4 + c[i]) ^ (c[i] >> 2);
        result4 = result4 * 3;
        /* Conditional based on computed value */
        if (result4 & 0x80) {
            result4 = result4 - 32;
        }
    }
    
    /* Loop 5: Short array processing with alternating operations */
    /* Creates another distinct scheduling pattern */
    for (int i = 0; i < n; ++i) {
        result5 = result5 + d[i];
        result5 = result5 - (d[i] >> 1);
        result5 = result5 | 0x1;
        /* Periodic reset to create phase changes */
        if ((i & 0x3F) == 0) {
            result5 = result5 & 0x7FFF;
        }
    }
    
    /* Combine all results to prevent elimination of any loop */
    int final_result = result1 + result2 + result3 + (int)result4 + (int)result5;
    
    /* Additional mixing to ensure all computations are used */
    final_result = final_result ^ (result1 >> 16);
    final_result = final_result * 31 + result2;
    final_result = final_result & 0x7FFFFFFF;
    
    return final_result;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    char *array_c = (char*)malloc(SIZE * sizeof(char));
    short *array_d = (short*)malloc(SIZE * sizeof(short));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink < 0) {
        /* This should never happen with our masking operations */
        __builtin_trap();
    }
    
    /* Print minimal output to avoid I/O dominating execution time */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
