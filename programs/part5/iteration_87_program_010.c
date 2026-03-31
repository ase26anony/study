/* Selective scheduler test program targeting sel-sched-dump.cc debug output */
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
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    short result4 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* Creates a dependency chain: result1 = ((result1 * a[i]) + b[i]) & 0xFF */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 & 0xFF;  /* Keep it bounded to prevent overflow */
    }
    
    /* Loop 2: Loop-carried dependency with condition */
    /* Uses different data types and conditional execution */
    int threshold = 500;
    for (int i = 0; i < size; ++i) {
        if (a[i] > threshold) {
            result2 += a[i] * 3;
        } else {
            result2 += b[i] * 2;
        }
        /* Additional operation to create more scheduling opportunities */
        result2 = (result2 << 1) | (result2 >> 31);  /* Simple rotation */
    }
    
    /* Loop 3: Multiple independent operations that can be reordered */
    /* Mix of char and short operations with promotion */
    for (int i = 0; i < size; ++i) {
        /* These operations have some dependencies but scheduler can reorder */
        int temp = (int)c[i] * (int)d[i];  /* Type promotion */
        result3 += temp;
        
        /* Independent operation that can be scheduled around the above */
        result3 ^= (b[i] & 0xF);  /* Mask operation */
        
        /* Another dependent operation */
        result3 = result3 + (a[i] % 16);
    }
    
    /* Loop 4: Short dependency chain with different operations */
    /* Uses short type for different instruction patterns */
    for (int i = 0; i < size; ++i) {
        result4 = (result4 + c[i]) * 2;
        result4 = result4 | (d[i] & 0x7F);  /* Mix of operations */
        
        /* Conditional based on loop variable */
        if ((i & 1) == 0) {
            result4 = result4 - (c[i] >> 2);
        }
    }
    
    /* Combine all results to prevent elimination of any loop */
    int final_result = result1 + result2 + result3 + (int)result4;
    
    /* Prevent dead code elimination without using volatile in loops */
    volatile int sink __attribute__((unused)) = final_result;
    
    return final_result;
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
    
    /* Call work function with array size as parameter to prevent constant propagation */
    int size = ARRAY_SIZE;
    int result = work(array_a, array_b, array_c, array_d, size);
    
    /* Use result to prevent elimination */
    if (result != 0) {  /* Result is non-deterministic due to pseudo-random inputs */
        printf("Result: %d\n", result);
    } else {
        /* This should never happen with our inputs, but ensures code isn't dead */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
