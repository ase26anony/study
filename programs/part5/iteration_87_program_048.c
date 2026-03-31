/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumps
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
    
    /* Loop 1: Tight loop with data-dependent chain (multiply-accumulate) */
    int sum1 = 0;
    for (int i = 0; i < n; ++i) {
        /* Creates dependency chain: sum1 -> sum1 * a[i] -> sum1 + b[i] */
        sum1 = (sum1 * a[i]) + b[i];
    }
    result ^= sum1;
    
    /* Loop 2: Mixed types with condition and promotion */
    int sum2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        /* char to int promotion, conditional execution */
        if (d[i] > (threshold % 128)) {
            /* Mixed operations create varied dependencies */
            sum2 += (int)c[i] * (a[i] & 0xFF);
        } else {
            sum2 |= b[i] ^ i;
        }
    }
    result += sum2;
    
    /* Loop 3: Independent parallel chains */
    int chain1 = 1, chain2 = 0;
    for (int i = 0; i < n; ++i) {
        /* Two independent dependency chains */
        chain1 = chain1 * 3 + a[i];
        chain2 = chain2 ^ b[i] - i;
    }
    result += chain1 - chain2;
    
    /* Loop 4: Short loop with multiple uses of same variable */
    int acc = 0;
    for (int i = 0; i < n; i += 2) {
        if (i + 1 < n) {
            /* Multiple uses of 'acc' create scheduling opportunities */
            temp1 = acc + a[i];
            temp2 = acc * b[i+1];
            acc = temp1 ^ temp2;
        }
    }
    result ^= acc;
    
    /* Loop 5: Search loop with early exit possibility */
    int found = 0;
    int target = 750;
    for (int i = 0; i < n && !found; ++i) {
        /* Data-dependent control flow */
        if (a[i] > target && b[i] < target) {
            found = 1;
            result += i * 1000;
        }
    }
    
    return result;
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
    
    /* Call work function with arrays - prevents constant propagation */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* Should never happen */
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
