/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
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
    int result = 0;
    int temp1 = 0, temp2 = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Loop 1: Mixed operations with loop-carried dependency */
    for (int i = 0; i < size; ++i) {
        /* Multiple uses of result with different operations */
        result = (result * a[i]) + b[i];
        /* Additional operation to create more scheduling opportunities */
        temp1 = temp1 ^ (a[i] & b[i]);
    }
    
    /* Loop 2: Conditional operations with mixed types */
    int threshold = 500;
    short scale = 3;
    for (int i = 0; i < size; i++) {
        /* Condition creates potential for speculative execution */
        if (a[i] > threshold) {
            /* Mixed type operations: int * short -> int */
            temp2 += a[i] * scale;
        }
        /* Always execute path */
        temp_short += c[i];
    }
    
    /* Loop 3: Data-dependent loop with multiple modifications */
    int acc = 1;
    for (int i = 0; i < size; ++i) {
        /* Chain of dependent operations */
        acc = (acc << 1) | (d[i] & 0x1F);
        /* Independent operation in same iteration */
        temp_char ^= d[i];
        /* Another dependent operation */
        result += acc;
    }
    
    /* Loop 4: Reduction with multiple dependency chains */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < size; ++i) {
        /* Two independent chains in same loop */
        sum1 = sum1 + (a[i] | b[i]);
        sum2 = sum2 + (a[i] & b[i]);
        /* Cross-chain operation every 8 iterations */
        if ((i & 7) == 0) {
            sum1 ^= sum2;
        }
    }
    
    /* Combine all results to prevent elimination */
    result = result + temp1 + temp2 + temp_short + temp_char + sum1 + sum2 + acc;
    return result;
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
    
    /* Call work function - this is where selective scheduling should occur */
    int result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Print result to prevent complete optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
