/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static inline unsigned int simple_rand(unsigned int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, char *c, short *d, unsigned int seed) {
    for (int i = 0; i < SIZE; i++) {
        a[i] = (int)(simple_rand(&seed) % 100);
        b[i] = (int)(simple_rand(&seed) % 100);
        c[i] = (char)(simple_rand(&seed) % 128);
        d[i] = (short)(simple_rand(&seed) % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int n) {
    int result1 = 0, result2 = 0, result3 = 0;
    char result4 = 0;
    
    /* Loop 1: Multiplicative-accumulative with loop-carried dependency */
    /* sum = (sum * a[i]) + b[i] */
    for (int i = 0; i < n; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed operations with conditional update */
    /* Uses different data types (int, char) */
    for (int i = 0; i < n; i++) {
        int temp = a[i] + (int)c[i];
        if (temp > 100) {
            result2 += temp * 3;
        } else {
            result2 += temp & 0x7F;
        }
    }
    
    /* Loop 3: Short dependency chain with bitwise operations */
    /* result3 = (result3 ^ a[i]) | b[i] */
    for (int i = 0; i < n; i += 2) {
        result3 = (result3 ^ a[i]) | b[i];
        if (i + 1 < n) {
            result3 = (result3 & a[i + 1]) ^ b[i + 1];
        }
    }
    
    /* Loop 4: Character processing with promotion/demotion */
    /* Mix of char and int operations */
    for (int i = 0; i < n; i++) {
        char val = c[i];
        if (val > 64) {
            result4 += (val - 32) * 2;
        } else {
            result4 |= val & 0x1F;
        }
        /* Small dependency chain within loop */
        result4 = (result4 << 1) | (result4 >> 7);
    }
    
    /* Loop 5: Short loop with array transformation */
    /* Creates independent but interleaved operations */
    for (int i = 1; i < n - 1; i++) {
        d[i] = (short)((d[i-1] + d[i] + d[i+1]) / 3);
        a[i] = a[i] ^ (a[i-1] & 0xFF);
    }
    
    /* Combine all results */
    return result1 + result2 + result3 + (int)result4;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    char *array_c = (char*)malloc(SIZE * sizeof(char));
    short *array_d = (short*)malloc(SIZE * sizeof(short));
    
    unsigned int seed = 42;
    init_arrays(array_a, array_b, array_c, array_d, seed);
    
    /* Call work function with multiple loops */
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
