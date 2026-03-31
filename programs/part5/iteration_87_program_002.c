/* sel-sched-trigger.c */
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
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (char)(lcg_rand() % 128);
        d[i] = (short)(lcg_rand() % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int n) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Multiplicative-accumulate with loop-carried dependency */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed operations with data-dependent condition */
    int threshold = 50;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            result2 += a[i] * b[i];
        } else {
            result2 -= b[i];
        }
    }
    
    /* Loop 3: Chain of dependent operations with mixed types */
    char acc_char = 0;
    for (int i = 0; i < n; i++) {
        /* Mix char and int operations */
        int temp = (int)c[i] * 2;
        acc_char = (char)((acc_char + temp) & 0x7F);
        result3 += (int)acc_char * d[i];
    }
    
    /* Loop 4: Independent parallel chains */
    int chain_a = 1;
    int chain_b = 0;
    for (int i = 0; i < n; i++) {
        chain_a = chain_a ^ (a[i] + i);
        chain_b = chain_b | (b[i] - i);
    }
    result4 = chain_a + chain_b;
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
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
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Clean up */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
