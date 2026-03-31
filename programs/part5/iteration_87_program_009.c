/* sel-sched-test.c - Program to trigger selective scheduler debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
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
    int result = 0;
    int temp1 = 0, temp2 = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    for (int i = 0; i < n; ++i) {
        /* Chain: use -> modify -> use pattern */
        temp1 = (temp1 * a[i]) + b[i];
        result ^= temp1;  /* Mix result */
    }
    
    /* Loop 2: Mixed-type operations with condition */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        /* Mix int and short types */
        int val = (int)c[i] * 2;
        if (val > threshold) {
            temp2 += val * a[i];  /* Data-dependent multiply */
        } else {
            temp2 -= b[i];        /* Alternative path */
        }
        result += temp2;  /* Accumulate */
    }
    
    /* Loop 3: Bitwise operations with char promotion */
    for (int i = 0; i < n; ++i) {
        /* Promote char to int, create dependency chain */
        temp_char = (temp_char & d[i]) | (char)(i & 0xFF);
        temp_short = (short)(temp_char * 3) + c[i];
        result |= (int)temp_short;  /* Combine with bitwise OR */
    }
    
    /* Loop 4: Nested dependency with multiple uses */
    int acc = 0;
    for (int i = 1; i < n - 1; ++i) {
        /* Multiple uses of same variables */
        int x = a[i-1] + b[i];
        int y = a[i] * b[i+1];
        acc = (acc + x) * y;  /* Complex dependency */
        result += acc;
    }
    
    /* Combine all results */
    return result + temp1 + temp2;
}

int main(int argc, char **argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int n = (argc > 1) ? atoi(argv[1]) : 200;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(n * sizeof(int));
    int *b = (int*)malloc(n * sizeof(int));
    short *c = (short*)malloc(n * sizeof(short));
    char *d = (char*)malloc(n * sizeof(char));
    
    if (!a || !b || !c || !d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(a, b, c, d, n);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(a, b, c, d, n);
    
    /* Prevent dead code elimination with volatile sink */
    volatile int sink = result;
    
    /* Use result in side effect to ensure code isn't removed */
    if (sink != 0xDEADBEEF) {  /* Arbitrary check */
        printf("Result: %d\n", sink);
    }
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    
    return 0;
}
