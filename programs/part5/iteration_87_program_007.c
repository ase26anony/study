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
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *arr1, int *arr2, short *arr3, char *arr4, int n) {
    for (int i = 0; i < n; i++) {
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
    /* Creates a tight dependency chain: result1 = ((result1 * a[i]) + b[i]) */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    /* int + short * char operations with condition */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            result2 += a[i] * (int)c[i % n];
        } else {
            result2 += b[i] | (int)d[i % n];
        }
    }
    
    /* Loop 3: Independent parallel chains */
    /* Two separate dependency chains in same loop */
    int chain1 = 1, chain2 = 0;
    for (int i = 0; i < n; ++i) {
        chain1 = chain1 * 3 + a[i];
        chain2 = chain2 ^ b[i] + i;
    }
    result3 = chain1 + chain2;
    
    /* Loop 4: Reduction with varying stride and condition */
    /* Creates complex scheduling decisions with stride access */
    for (int i = 0; i < n; i += 2) {
        int idx = i % n;
        if (c[idx] > 100) {
            result4 += (a[idx] & 0xFF) * (int)d[idx];
        } else {
            result4 += (b[idx] | 0x3F) - (int)c[idx];
        }
    }
    
    /* Loop 5: Short loop-carried dependency with multiple uses */
    /* Variable used, modified, and used again next iteration */
    int temp = result1;
    for (int i = 0; i < n; ++i) {
        temp = (temp + a[i]) * (temp & b[i]);
        result4 = result4 ^ temp;
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    short *array3 = (short*)malloc(SIZE * sizeof(short));
    char *array4 = (char*)malloc(SIZE * sizeof(char));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, array4, SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int final_result = work(array1, array2, array3, array4, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
