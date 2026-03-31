/* Selective scheduling test case targeting sel-sched-dump.cc debug output */
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
    int temp1, temp2;
    char char_temp;
    short short_temp;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* sum = (sum * a[i]) + b[i] pattern */
    temp1 = 1;
    for (int i = 0; i < n; ++i) {
        temp1 = (temp1 * a[i]) + b[i];
        result1 ^= temp1;  /* Use XOR to combine results */
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* Uses char type with promotion */
    temp2 = 0;
    for (int i = 0; i < n; i++) {
        char_temp = c[i];
        if (char_temp > 100) {  /* Simple condition */
            temp2 += (int)char_temp * (i & 0xF);  /* Mix with loop index */
        } else {
            temp2 -= (int)char_temp | 0x1;  /* Different operation */
        }
        result2 += temp2;
    }
    
    /* Loop 3: Multiple independent chains in same loop */
    /* Short type with arithmetic and bitwise ops */
    int chain1 = 0, chain2 = 0;
    for (int i = 0; i < n; i++) {
        short_temp = d[i];
        
        /* First dependency chain */
        chain1 = (chain1 + short_temp) * 3;
        
        /* Second independent chain */
        chain2 = (chain2 ^ (short_temp & 0xFF)) + (i % 8);
        
        /* Combine chains with different operations */
        result3 += chain1 - chain2;
    }
    
    /* Loop 4: Nested dependency with multiple uses */
    /* More complex data flow for scheduler */
    int acc = result1;
    for (int i = 1; i < n - 1; i++) {
        /* Multiple uses of acc with array accesses */
        acc = (acc & a[i]) | (acc + b[i-1]);
        acc = acc ^ (acc * 2);
        result3 = result3 + acc + c[i] - d[i+1];
    }
    
    /* Final combination to prevent elimination */
    return result1 + result2 * 3 - result3;
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
    
    /* Call work function with arrays */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return 0;
}
