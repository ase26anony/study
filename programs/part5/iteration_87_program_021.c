/* sel-sched-trigger.c - Program to trigger selective scheduling debug dumps */
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
    int result = 0;
    int temp1 = 0, temp2 = 0;
    short temp_short = 0;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    for (int i = 0; i < n; ++i) {
        /* Creates dependency chain: use -> modify -> use */
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;  /* Mix result */
    }
    
    /* Loop 2: Mixed-type operations with condition */
    int threshold = 500;
    for (int i = 0; i < n; i++) {
        /* Mix int and short types, creating promotion instructions */
        int val = (int)c[i] * 3;
        if (val > threshold) {
            /* Data-dependent operation with multiple uses */
            temp2 = (temp2 & val) | (temp2 + d[i]);
            result += temp2;
        } else {
            temp2 = temp2 - (val >> 2);
        }
    }
    
    /* Loop 3: Independent accumulation with bitwise operations */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < n; i += 2) {
        /* Two independent chains in same loop */
        sum1 = sum1 ^ (a[i] * 2);
        sum2 = sum2 | (b[i] + 1);
        
        /* Cross-dependency between chains */
        temp_short = (short)((sum1 & 0xFF) + (sum2 & 0xFF));
        result += (int)temp_short;
    }
    
    /* Loop 4: Search with early exit possibility */
    int target = 750;
    int found_idx = -1;
    for (int i = 0; i < n; i++) {
        /* Creates potential for speculative execution */
        if (a[i] > target && found_idx == -1) {
            found_idx = i;
            result += found_idx * 10;
        }
        /* Continue computation even after finding */
        temp1 = (temp1 + b[i]) * 17;
    }
    
    /* Combine all results */
    result = result + temp1 + temp2 + sum1 + sum2;
    return result;
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
    
    /* Call work function with selective scheduling opportunities */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = final_result;
    
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
