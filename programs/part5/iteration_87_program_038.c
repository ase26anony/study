/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with deterministic values */
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
    int temp1, temp2;
    short temp_short;
    char temp_char;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    /* Creates a dependency chain: result1 = ((result1 * a[i]) + b[i]) & 0xFFF */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
        result1 = result1 & 0xFFF;  /* Keep it bounded */
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    /* int + short * char operations with condition */
    int threshold = 500;
    for (int i = 0; i < size; ++i) {
        temp1 = (int)c[i] * (int)d[i];  /* short * char promoted to int */
        if (temp1 > threshold) {
            result2 += temp1 * a[i];  /* Data-dependent on previous loop's a[i] */
        } else {
            result2 += b[i] | 0x1;    /* Different operation path */
        }
    }
    
    /* Loop 3: Independent reduction with bitwise operations */
    /* Separate dependency chain from previous loops */
    temp2 = result1 ^ result2;  /* Mix previous results */
    for (int i = 0; i < size; ++i) {
        temp2 = (temp2 << 1) | (temp2 >> 31);  /* Rotate left */
        temp2 = temp2 ^ (a[i] & b[i]);         /* XOR with combined inputs */
        if (i % 3 == 0) {
            temp2 += c[i] * 2;  /* Periodic additional operation */
        }
    }
    result3 = temp2;
    
    /* Loop 4: Search loop with early exit possibility */
    /* Gives scheduler multiple paths to consider */
    int search_key = 750;
    int found_index = -1;
    for (int i = 0; i < size && found_index < 0; ++i) {
        if (a[i] > search_key && b[i] < search_key) {
            found_index = i;
            result3 += found_index * 1000;
        }
    }
    
    /* Combine all results */
    return (result1 ^ result2) + result3;
}

int main(void) {
    const int size = 256;  /* Medium size for scheduling opportunities */
    
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(size * sizeof(int));
    int *array_b = (int*)malloc(size * sizeof(int));
    short *array_c = (short*)malloc(size * sizeof(short));
    char *array_d = (char*)malloc(size * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, size);
    
    /* Call work function with all loops */
    int final_result = work(array_a, array_b, array_c, array_d, size);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = final_result;
    
    /* Simple side effect to keep the result alive */
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
