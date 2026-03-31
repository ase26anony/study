/* sel-sched-trigger.c
 * Program to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdint.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static uint32_t lcg(uint32_t x) {
    return (1103515245 * x + 12345) & 0x7fffffff;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, char *c, short *d, int size, int seed) {
    uint32_t state = seed;
    for (int i = 0; i < size; i++) {
        state = lcg(state);
        a[i] = (int)(state % 100);
        state = lcg(state);
        b[i] = (int)(state % 100);
        state = lcg(state);
        c[i] = (char)(state % 128);
        state = lcg(state);
        d[i] = (short)(state % 256);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight loop with data-dependent chain (multiplication and addition) */
    /* Creates RAW dependencies: result1 -> result1 -> result1 */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Mixed operations with loop-carried dependency */
    /* Uses different operations to create varied dependency patterns */
    for (int i = 0; i < size; ++i) {
        result2 = (result2 & a[i]) | (result2 + b[i]);
        result2 ^= (result2 << 3) | (result2 >> 5);
    }
    
    /* Loop 3: Conditional loop with mixed data types */
    /* char and short types create promotion/demotion instructions */
    int threshold = 50;
    for (int i = 0; i < size; i++) {
        if (c[i] > threshold) {
            result3 += (int)c[i] * (int)d[i];
        } else {
            result3 -= (int)c[i] | (int)d[i];
        }
    }
    
    /* Loop 4: Nested dependency chain with simple condition */
    /* Multiple uses of same variable in same iteration */
    int last = 0;
    for (int i = 0; i < size; ++i) {
        int temp = a[i] + b[i];
        if (temp > 100) {
            last = (last * 3) + temp;
        } else {
            last = (last / 2) - temp;
        }
        result4 += last;
    }
    
    /* Loop 5: Independent parallel chains */
    /* Gives scheduler multiple independent operations to reorder */
    int chain1 = 0, chain2 = 0, chain3 = 0;
    for (int i = 0; i < size; i += 2) {
        chain1 = chain1 * 7 + a[i];
        chain2 = chain2 * 11 + b[i];
        chain3 = chain3 * 13 + (int)c[i];
    }
    result4 += chain1 + chain2 + chain3;
    
    /* Combine all results */
    return result1 + result2 + result3 + result4;
}

int main(void) {
    /* Declare arrays with different types */
    int array_a[ARRAY_SIZE];
    int array_b[ARRAY_SIZE];
    char array_c[ARRAY_SIZE];
    short array_d[ARRAY_SIZE];
    
    /* Initialize with deterministic values */
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE, 42);
    
    /* Call work function - this is where selective scheduling should occur */
    int final_result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Prevent dead code elimination without using volatile in loops */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink != 0) {
        printf("Result: %d\n", sink);
    } else {
        printf("Zero result\n");
    }
    
    return 0;
}
