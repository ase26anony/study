/* sel-sched-trigger.c - Program to trigger selective scheduling debug dumps */
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
    int temp1 = 0, temp2 = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < n; ++i) {
        /* Chain: use -> modify -> use pattern */
        temp1 = (temp1 * a[i]) + b[i];
        temp1 = temp1 ^ (a[i] & 0xFF);
        result += temp1;
    }
    
    /* Loop 2: Conditional operations with type mixing */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            /* Mix int and short types */
            temp_short = (short)(c[i] * 2);
            temp2 += a[i] * (int)temp_short;
        } else {
            temp2 -= b[i] / 3;
        }
        /* Additional operation to create scheduling choices */
        temp2 = temp2 | (b[i] & 0x7F);
    }
    result ^= temp2;
    
    /* Loop 3: Nested dependency with char type */
    int limit = n - 1;
    for (int i = 1; i < limit; i++) {
        /* Create data flow between iterations */
        temp_char = (char)((d[i-1] + d[i] + d[i+1]) / 3);
        int scaled = (int)temp_char * 17;
        
        /* Conditional with arithmetic */
        if (scaled > 1000) {
            result += scaled >> 2;
        } else {
            result -= scaled << 1;
        }
        
        /* Another dependent operation */
        temp_char = (char)(temp_char ^ (d[i] & 0x3F));
    }
    
    /* Loop 4: Multiple independent chains */
    int chain1 = 0, chain2 = 0;
    for (int i = 0; i < n; i += 2) {
        /* First dependency chain */
        chain1 = (chain1 + a[i]) * 3;
        chain1 = chain1 ^ (b[i] | 0x55);
        
        /* Second independent chain */
        if (i + 1 < n) {
            chain2 = (chain2 - b[i+1]) & 0xFFF;
            chain2 = chain2 + (a[i+1] % 64);
        }
        
        /* Interaction between chains */
        result += chain1 - chain2;
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
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function with array size to prevent constant propagation */
    int size = SIZE;
    int result = work(array_a, array_b, array_c, array_d, size);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = result;
    
    /* Use result to prevent removal */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Clean up */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
