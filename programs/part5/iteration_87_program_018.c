/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
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
    int result = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    char char_sum = 0;
    short short_prod = 1;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < size; ++i) {
        /* Chain: use -> modify -> use pattern */
        temp1 = (temp1 * a[i]) + b[i];
        temp1 = temp1 ^ (a[i] & 0xFF);
        temp1 = temp1 | (b[i] << 3);
    }
    result ^= temp1;
    
    /* Loop 2: Conditional operations with mixed types */
    int threshold = 500;
    for (int i = 0; i < size; ++i) {
        /* Mix int and char operations with condition */
        if (a[i] > threshold) {
            char_sum += (char)(c[i] * 2);
            temp2 += a[i] * (int)char_sum;
        } else {
            temp2 -= b[i] / 3;
        }
        /* Additional operation to create scheduling choices */
        temp2 = (temp2 & 0xFFFF) | (temp2 << 16);
    }
    result += temp2;
    
    /* Loop 3: Short type operations with loop-carried dependency */
    for (int i = 0; i < size; ++i) {
        /* Multiple uses of short_prod with promotion */
        short_prod = (short_prod * (short)(d[i] % 100)) + 1;
        temp3 += (int)short_prod * (i & 0xF);
        
        /* Another dependent operation */
        if (short_prod > 1000) {
            short_prod = (short_prod >> 1) | 0x80;
        }
    }
    result ^= temp3;
    
    /* Loop 4: Complex dependency pattern with multiple chains */
    int chain1 = 1, chain2 = 0x5A5A5A5A;
    for (int i = 0; i < size; i += 2) {
        /* Two independent chains that get combined */
        chain1 = (chain1 + a[i]) * 3;
        chain2 = chain2 ^ (b[i] * chain1);
        
        /* Cross-chain dependency */
        int combined = chain1 & chain2;
        result += combined % 100;
        
        /* Additional operation on the second element */
        if (i + 1 < size) {
            chain1 = (chain1 - a[i + 1]) | 1;
            chain2 = chain2 ^ (b[i + 1] << (i % 8));
        }
    }
    
    return result;
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
    int result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = result;
    
    /* Use result in side effect */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* Should never happen */
    }
    
    /* Print minimal output to prevent optimization */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
