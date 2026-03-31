/* sel-sched-test.c - Program to trigger selective scheduling debug output */
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
    int i;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    /* Creates: result1 = ((result1 * a[i]) + b[i]) & 0xFF */
    for (i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 & 0xFF;  /* Keep within byte range */
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    /* int + char * short operations */
    int threshold = 5000;
    for (i = 0; i < n; i++) {
        if (d[i] > threshold) {
            result2 += (int)c[i] * (int)d[i];
        } else {
            result2 += (int)c[i] | (int)d[i];
        }
    }
    
    /* Loop 3: Multiple independent dependency chains */
    int chain1 = 1, chain2 = 0, chain3 = 0xFFFF;
    for (i = 0; i < n; i++) {
        /* First chain: multiplicative */
        chain1 = (chain1 * (a[i] & 0xF)) + 1;
        
        /* Second chain: additive with condition */
        chain2 = chain2 + b[i];
        if (chain2 > 1000000) {
            chain2 = chain2 >> 1;
        }
        
        /* Third chain: bitwise operations */
        chain3 = chain3 ^ (c[i] << (i % 8));
    }
    result3 = chain1 + chain2 + chain3;
    
    /* Loop 4: Short loop with data-dependent exit */
    int search_val = 42;
    int found_at = -1;
    for (i = 0; i < n && found_at < 0; i++) {
        if (c[i] == search_val) {
            found_at = i;
        }
    }
    
    /* Combine all results */
    return result1 ^ result2 ^ result3 ^ found_at;
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
    
    /* Call work function - this should trigger selective scheduling */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
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
