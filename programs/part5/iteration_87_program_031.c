/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 123456789;
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
    int result1 = 0;
    int result2 = 0;
    short result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* Creates: sum = (sum * a[i]) + b[i] pattern */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
        /* Add another dependent operation to lengthen chain */
        result1 = result1 ^ (a[i] & 0xFF);
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* if (data[i] > threshold) { total += data[i] * scale; } pattern */
    int threshold = 500;
    int scale = 3;
    for (int i = 0; i < size; ++i) {
        if (a[i] > threshold) {
            result2 += a[i] * scale;
        } else {
            result2 += b[i] / 2;
        }
        /* Additional dependent operation */
        result2 = result2 | (i & 0xF);
    }
    
    /* Loop 3: Mixed data types with promotion/demotion */
    /* short and char types create conversion instructions */
    for (int i = 0; i < size; ++i) {
        /* Mix types to create promotion instructions */
        int temp = (int)c[i] * (int)d[i];
        result3 += (short)(temp & 0xFFFF);
        
        /* Another dependent operation with type mixing */
        result3 = (short)(result3 + (c[i] >> 1));
    }
    
    /* Loop 4: Multiple independent operations that can be reordered */
    /* Gives scheduler choices for instruction placement */
    for (int i = 0; i < size; ++i) {
        /* Three independent chains that could be scheduled in parallel */
        int chain1 = a[i] * 7;
        int chain2 = b[i] + 11;
        short chain3 = c[i] - 5;
        
        /* Then combine them with dependencies */
        result4 += chain1 + chain2 + (int)chain3;
        
        /* Additional operation that depends on combined result */
        result4 = result4 ^ (i * 2);
    }
    
    /* Combine all results to prevent elimination */
    int final_result = result1 + result2 + (int)result3 + result4;
    
    /* Add some final mixing */
    final_result = final_result ^ (result1 * result2);
    final_result = final_result | ((int)result3 << 16);
    
    return final_result;
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
    
    /* Call work function with array size to prevent constant propagation */
    int size = ARRAY_SIZE;
    int result = work(array_a, array_b, array_c, array_d, size);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Print to prevent elimination and verify execution */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
