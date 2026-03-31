/* sel-sched-trigger.c - Program to trigger selective scheduler debug dumps */
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
    short result4 = 0;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
        if (result1 < 0) {
            result1 = -result1;
        }
    }
    
    /* Loop 2: Conditional accumulation with type mixing */
    int threshold = 500;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            result2 += a[i] * (int)c[i];
        } else {
            result2 += b[i] / (d[i] + 1);
        }
    }
    
    /* Loop 3: Independent chain with bit operations */
    for (int i = 0; i < n; i += 2) {
        int temp = a[i] & b[i];
        result3 = (result3 | temp) ^ (result3 << 1);
        if (i + 1 < n) {
            temp = a[i+1] | b[i+1];
            result3 = (result3 & temp) + (result3 >> 2);
        }
    }
    
    /* Loop 4: Short dependency chain with char/short types */
    for (int i = 0; i < n; i++) {
        short val = c[i] * (short)d[i];
        result4 = (result4 + val) - (result4 >> 3);
        if (val > 1000) {
            result4 = result4 * 2;
        }
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + (int)result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *arr_a = (int*)malloc(SIZE * sizeof(int));
    int *arr_b = (int*)malloc(SIZE * sizeof(int));
    short *arr_c = (short*)malloc(SIZE * sizeof(short));
    char *arr_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!arr_a || !arr_b || !arr_c || !arr_d) {
        return 1;
    }
    
    init_arrays(arr_a, arr_b, arr_c, arr_d, SIZE);
    
    /* Call work function with arrays */
    int final_result = work(arr_a, arr_b, arr_c, arr_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(arr_a);
    free(arr_b);
    free(arr_c);
    free(arr_d);
    
    return 0;
}
