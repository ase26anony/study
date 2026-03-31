/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 123456789;
static inline unsigned int lcg_rand() {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *arr1, int *arr2, short *arr3, char *arr4, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = (short)(lcg_rand() % 256);
        arr4[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int n) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < n; ++i) {
        /* Create a dependency chain: use, modify, use */
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 ^ (a[i] & 0xFF);
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    for (int i = 0; i < n; ++i) {
        /* Mixed data types: char promoted to int */
        int val = (int)d[i] * 3;
        if (val > 100) {
            result2 += val * a[i];
        } else {
            result2 += val | b[i];
        }
        /* Another use of result2 creates dependency */
        result2 = result2 & 0x7FFFFFFF;
    }
    
    /* Loop 3: Independent operations that can be reordered */
    for (int i = 0; i < n; ++i) {
        /* Multiple independent calculations */
        int temp1 = a[i] + b[i];
        int temp2 = (int)c[i] * 2;
        int temp3 = temp1 * temp2;
        
        /* Data-dependent chain */
        result3 = result3 ^ temp3;
        result3 = result3 + (temp1 >> 2);
    }
    
    /* Loop 4: Short loop with multiple uses of same variable */
    for (int i = 0; i < n; ++i) {
        /* Create tight dependency chain */
        int x = a[i] + i;
        x = x * (b[i] + 1);
        x = x | (int)c[i];
        x = x ^ (int)d[i];
        result4 += x;
    }
    
    /* Combine all results */
    return result1 ^ result2 ^ result3 ^ result4;
}

int main(int argc, char **argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 256;
    
    /* Allocate arrays */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    short *arr3 = (short*)malloc(size * sizeof(short));
    char *arr4 = (char*)malloc(size * sizeof(char));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        return 1;
    }
    
    /* Initialize with deterministic values */
    init_arrays(arr1, arr2, arr3, arr4, size);
    
    /* Call work function with multiple scheduling opportunities */
    int final_result = work(arr1, arr2, arr3, arr4, size);
    
    /* Prevent dead code elimination without preventing scheduling */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {
        __builtin_trap();  /* Unlikely to execute */
    }
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
