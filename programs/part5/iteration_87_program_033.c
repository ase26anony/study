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
static void init_arrays(int *a, int *b, char *c, short *d, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (char)(lcg_rand() % 256);
        d[i] = (short)(lcg_rand() % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int n) {
    int result1 = 0, result2 = 0, result3 = 0;
    int temp1, temp2;
    char char_temp;
    short short_temp;
    
    /* Loop 1: Mixed operations with loop-carried dependency */
    /* sum = (sum * a[i]) + b[i] pattern */
    temp1 = 1;
    for (int i = 0; i < n; ++i) {
        temp1 = (temp1 * a[i]) + b[i];
        result1 ^= temp1;  /* Use XOR to combine results */
    }
    
    /* Loop 2: Conditional accumulation with mixed types */
    /* if (data[i] > threshold) { total += data[i] * scale; } */
    int threshold = 500;
    int scale = 3;
    temp2 = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            temp2 += a[i] * scale;
        }
        /* Mix in char type operations */
        char_temp = c[i];
        if (char_temp > 128) {
            temp2 -= (int)char_temp;
        }
    }
    result2 = temp2;
    
    /* Loop 3: Short type operations with bitwise mixing */
    short_temp = 0;
    for (int i = 0; i < n; i++) {
        short_temp = (short_temp & d[i]) | (short)(i & 0xFF);
        /* Create data-dependent chain */
        short_temp = short_temp + d[i] - (short)(i % 64);
    }
    result3 = (int)short_temp;
    
    /* Loop 4: Independent parallel-like operations */
    /* Gives scheduler multiple independent chains to work with */
    int chain1 = 0, chain2 = 0, chain3 = 0;
    for (int i = 0; i < n; i++) {
        chain1 = chain1 * 13 + a[i];
        chain2 = chain2 ^ b[i] + i;
        chain3 = chain3 | (int)c[i] << (i % 8);
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + chain1 + chain2 + chain3;
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
    
    /* Call work function - this is where selective scheduling should occur */
    int result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print minimal output to prevent optimization */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
