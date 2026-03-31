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
    int sum1 = 0, sum2 = 0, sum3 = 0;
    int i;
    
    /* Loop 1: Tight loop with data-dependent chain (multiply-accumulate) */
    for (i = 0; i < size; ++i) {
        sum1 = (sum1 * a[i]) + b[i];  /* Creates loop-carried dependency */
    }
    result ^= sum1;
    
    /* Loop 2: Mixed operations with condition */
    int threshold = 500;
    short scale = 3;
    for (i = 0; i < size; i++) {
        if (a[i] > threshold) {          /* Condition creates control flow */
            sum2 += a[i] * scale;        /* Mixed int/short operations */
        } else {
            sum2 += b[i] & 0xFF;         /* Different operation type */
        }
    }
    result ^= sum2;
    
    /* Loop 3: Multiple independent operations in loop body */
    char mask = 0x7F;
    for (i = 0; i < size; i++) {
        int temp = c[i] * 2;             /* short promotion to int */
        temp = temp | (d[i] & mask);     /* Mixed bitwise operations */
        sum3 += temp + (i & 1);          /* Loop-variant component */
    }
    result ^= sum3;
    
    /* Loop 4: Nested dependency chain with multiple uses */
    int chain = 1;
    for (i = 0; i < size; i++) {
        chain = (chain << 1) ^ a[i];     /* Shift then XOR */
        chain = chain + (b[i] >> 2);     /* Another dependent operation */
        result += chain & 0x3F;          /* Third use of chain */
    }
    
    return result;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    short *array_c = (short*)malloc(ARRAY_SIZE * sizeof(short));
    char *array_d = (char*)malloc(ARRAY_SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Call work function - this should trigger selective scheduling */
    int result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Prevent dead code elimination without preventing scheduling */
    volatile int sink = result;
    
    /* Use result to prevent optimization */
    if (sink != 0) {
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
