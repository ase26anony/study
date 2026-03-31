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
    int result1 = 0, result2 = 0, result3 = 0;
    short result4 = 0;
    
    /* Loop 1: Data-dependent chain with multiplication and addition */
    /* Creates tight dependency: result1 = (result1 * a[i]) + b[i] */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed types with condition and promotion */
    /* char -> int promotion, conditional accumulation */
    int threshold = 50;
    for (int i = 0; i < size; ++i) {
        if (d[i] > threshold) {
            result2 += d[i] * 3;  /* char promoted to int */
        }
    }
    
    /* Loop 3: Independent chain with bitwise operations */
    /* Multiple uses of result3 in same iteration */
    for (int i = 0; i < size; ++i) {
        int temp = c[i];  /* short promoted to int */
        result3 = (result3 & temp) | (result3 ^ (temp << 1));
        result3 += i & 0xF;  /* Additional dependency on loop index */
    }
    
    /* Loop 4: Short type loop with carry chain */
    /* Tight loop-carried dependency on result4 */
    for (int i = 0; i < size; ++i) {
        result4 = (result4 + c[i]) * 2;
        if (result4 > 1000) {
            result4 = result4 % 100;
        }
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
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
    
    /* Call work function with arrays - prevents constant propagation */
    int final_result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {  /* Unlikely but possible */
        __builtin_trap();
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
