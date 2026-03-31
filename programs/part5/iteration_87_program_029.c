/* Selective scheduler test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
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
    int temp1 = 1, temp2 = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    for (int i = 0; i < n; ++i) {
        /* Creates dependency chain: use -> modify -> use */
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;  /* Mix result */
    }
    
    /* Loop 2: Mixed-type operations with condition */
    for (int i = 0; i < n; ++i) {
        /* Promote char to int, use in conditional */
        int val = (int)d[i];
        if (val > 64) {  /* Data-dependent condition */
            temp2 += val * (int)c[i];  /* Mixed-type multiplication */
        } else {
            temp2 -= val | 0x1F;  /* Different operation for else path */
        }
    }
    result += temp2;
    
    /* Loop 3: Short-based loop with bitwise operations */
    for (int i = 0; i < n; ++i) {
        /* Chain of operations on short */
        temp_short = (temp_short & c[i]) | (temp_short ^ (short)i);
        temp_short += (short)(a[i] % 256);  /* Demotion from int */
        
        /* Nested dependency within loop iteration */
        for (int j = 0; j < 3; ++j) {
            temp_short = (temp_short << 1) | (temp_short >> 15);
        }
    }
    result ^= (int)temp_short;
    
    /* Loop 4: Search loop with early exit possibility */
    int found = 0;
    for (int i = 0; i < n && !found; ++i) {
        /* Complex condition with multiple uses */
        if ((a[i] > 500) && (b[i] < 300) && ((c[i] & 0x0F) == 0)) {
            temp_char = d[i] ^ (char)temp_short;
            found = 1;
        }
    }
    result += (int)temp_char;
    
    /* Loop 5: Reduction with stride to break simple patterns */
    for (int i = 0; i < n; i += 2) {
        /* Independent operations that could be scheduled together */
        int op1 = a[i] * 3;
        int op2 = b[i] / 2;
        result += op1 - op2;
        
        /* Additional operation creating local dependency */
        if (i + 1 < n) {
            result ^= (a[i + 1] & b[i + 1]);
        }
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
    
    /* Call work function - this should trigger selective scheduling */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
