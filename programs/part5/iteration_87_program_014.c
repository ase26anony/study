/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

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
    int result = 0;
    int temp1 = 0, temp2 = 0, temp3 = 0;
    short temp_short = 0;
    char temp_char = 0;
    
    /* Loop 1: Multiplicative-accumulate with loop-carried dependency */
    for (int i = 0; i < n; ++i) {
        temp1 = (temp1 * a[i]) + b[i];  /* Chain: use, modify, use */
        result ^= temp1;                /* Additional use */
    }
    
    /* Loop 2: Mixed operations with data-dependent condition */
    int threshold = 500;
    for (int i = 0; i < n; ++i) {
        if (a[i] > threshold) {         /* Condition for speculative execution */
            temp2 += (a[i] & 0xFF) | (b[i] & 0xFF00);  /* Mixed bitwise ops */
        } else {
            temp2 -= (a[i] % 64) * 2;   /* Different path */
        }
        /* Create short dependency chain within loop */
        temp_short = (short)(temp2 >> 1);
        temp2 += temp_short;
    }
    result += temp2;
    
    /* Loop 3: Nested dependency with mixed types */
    for (int i = 0; i < n; ++i) {
        /* Promote char to int, operate, demote */
        int promoted = (int)d[i];
        temp3 = (temp3 + promoted) * 3;
        
        /* Use short in calculation */
        temp3 += (int)c[i] * 2;
        
        /* Create alternating pattern */
        if (i & 1) {
            temp3 &= 0xFFFF;
        } else {
            temp3 |= 0x10000;
        }
    }
    result ^= temp3;
    
    /* Loop 4: Independent parallel-like computations */
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < n; i += 2) {
        /* Two independent chains in same loop */
        sum1 = sum1 * 7 + a[i];
        sum2 = sum2 * 11 + b[i + 1];
        
        /* Cross-dependency occasionally */
        if ((i % 8) == 0) {
            sum1 ^= sum2;
            sum2 ^= sum1;
        }
    }
    result += sum1 + sum2;
    
    /* Loop 5: Search with early exit possibility */
    int found = 0;
    int target = 750;
    for (int i = 0; i < n && !found; ++i) {
        if (a[i] == target) {
            found = 1;
            result += i * 1000;
        }
        /* Continue computation even when not found */
        temp_char = (char)((d[i] + i) & 0x7F);
        result += temp_char;
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
    
    /* Call work function - this is where selective scheduling happens */
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
