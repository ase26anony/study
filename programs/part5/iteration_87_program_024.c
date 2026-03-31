/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
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
    int i;
    
    /* Loop 1: Multiply-accumulate with data-dependent chain */
    for (i = 0; i < n; ++i) {
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;
    }
    
    /* Loop 2: Mixed operations with char/short promotion */
    for (i = 0; i < n; ++i) {
        short sval = c[i];
        char cval = d[i];
        /* Mixed-width operations create promotion/demotion */
        temp2 = temp2 + (sval * cval) - (sval & cval);
        if (temp2 > 1000) {
            result += temp2;
            temp2 = temp2 / 2;
        }
    }
    
    /* Loop 3: Conditional accumulation with simple condition */
    int threshold = 500;
    int count = 0;
    for (i = 0; i < n; ++i) {
        if (a[i] > threshold) {
            count++;
            result = result | (a[i] & b[i]);
        } else {
            result = result ^ (a[i] | b[i]);
        }
    }
    
    /* Loop 4: Independent computation with different dependency pattern */
    int sum1 = 0, sum2 = 0;
    for (i = 0; i < n; i += 2) {
        sum1 = sum1 + a[i] * 3;
        sum2 = sum2 + b[i + 1] * 7;
        /* Cross-iteration dependency through result */
        result = (result + sum1) * 2 - sum2;
    }
    
    /* Loop 5: Short loop with bit manipulation */
    int mask = 0xFF;
    for (i = n - 1; i >= 0; --i) {
        int val = a[i] & mask;
        mask = (mask << 1) | (val & 1);
        result = result ^ (val * mask);
    }
    
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
    
    /* Call work function with arrays */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
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
    
    return 0;
}
