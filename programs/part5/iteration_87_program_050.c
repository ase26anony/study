/* Selective scheduler trigger program for sel-sched-dump.cc coverage */
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
    int result1 = 0, result2 = 0, result3 = 0;
    int temp1, temp2;
    short temp_short;
    char temp_char;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    /* Creates: RAW dependency chain with multiple uses */
    temp1 = 1;
    for (int i = 0; i < n; ++i) {
        /* Multiple uses of temp1 with different operations */
        temp1 = (temp1 * a[i]) + b[i];
        temp1 = temp1 ^ (temp1 >> 3);
        temp1 = temp1 & 0x7FFF;
        result1 += temp1;
    }
    
    /* Loop 2: Conditional operations with type mixing */
    /* Creates: Control flow and type conversion opportunities */
    temp2 = 0;
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            /* Mix int and short operations */
            temp_short = (short)(c[i] * 2);
            temp2 += (int)temp_short * b[i];
        } else {
            /* Different path with char operations */
            temp_char = d[i] + 1;
            temp2 -= (int)temp_char;
        }
        /* Additional operation to extend dependency chain */
        temp2 = (temp2 * 3) / 2;
    }
    result2 = temp2;
    
    /* Loop 3: Independent parallel chains */
    /* Creates: Multiple independent dep chains for scheduling */
    int chain1 = 0, chain2 = 0;
    for (int i = 0; i < n; i += 2) {
        /* First dependency chain */
        chain1 = chain1 * a[i] + b[i];
        chain1 = chain1 | (chain1 << 1);
        
        /* Second independent chain */
        if (i + 1 < n) {
            chain2 = chain2 ^ (a[i + 1] & b[i + 1]);
            chain2 = chain2 + (int)c[i + 1];
        }
        
        /* Interaction between chains */
        if ((chain1 & 1) == 0) {
            chain2 = chain2 ^ chain1;
        }
    }
    result3 = chain1 + chain2;
    
    /* Loop 4: Short loop with complex expression */
    /* Creates: Tight loop with multiple operation types */
    int sum = 0;
    for (int i = 0; i < n && i < 100; ++i) {
        /* Complex expression with multiple dependencies */
        int val = a[i] * 2 - b[i];
        val = (val > 0) ? val : -val;
        sum += val * (int)d[i];
        sum = sum % 10007;  /* Prevent overflow, creates div/mod ops */
    }
    
    /* Combine all results */
    return result1 ^ result2 ^ result3 ^ sum;
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
    
    /* Call work function - this is where selective scheduling happens */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = final_result;
    
    /* Use result to prevent optimization */
    if (sink != 0xDEADBEEF) {  /* Arbitrary check */
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
