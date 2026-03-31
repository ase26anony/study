/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

#define ARRAY_SIZE 256

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with deterministic pseudo-random values */
static void init_arrays(int *a, int *b, char *c, short *d, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 1000);
        b[i] = (int)(lcg_rand() % 1000);
        c[i] = (char)(lcg_rand() % 256);
        d[i] = (short)(lcg_rand() % 1000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, char *c, short *d, int size) {
    int result1 = 0, result2 = 0, result3 = 0;
    int temp1, temp2;
    char char_temp;
    short short_temp;
    
    /* Loop 1: Data-dependent chain with mixed operations */
    /* Creates a long dependency chain that scheduler might try to break */
    temp1 = a[0];
    for (int i = 0; i < size; i++) {
        /* Multiple uses of temp1 with different operations */
        temp1 = (temp1 * a[i]) + b[i];      /* Multiply-add chain */
        temp1 = temp1 ^ (temp1 >> 3);       /* Non-linear operation */
        temp1 = temp1 & 0x7FFFFFFF;         /* Mask operation */
        result1 += temp1;                   /* Accumulate */
    }
    
    /* Loop 2: Loop-carried dependency with condition and type mixing */
    /* Uses char and short types to create promotion/demotion instructions */
    temp2 = 1;
    for (int i = 0; i < size; i++) {
        char_temp = c[i];
        short_temp = d[i];
        
        /* Condition creates potential for speculative execution */
        if (char_temp > 128) {
            /* Mixed-type arithmetic */
            temp2 = temp2 * (int)char_temp + (int)short_temp;
            temp2 = (temp2 + i) & 0xFFF;    /* Modulo-like operation */
        } else {
            temp2 = temp2 - (int)char_temp + (int)short_temp;
        }
        
        /* Additional arithmetic to create scheduling choices */
        result2 ^= temp2;                   /* XOR creates data dependencies */
        temp2 = temp2 + (i & 0xF);          /* Small increment */
    }
    
    /* Loop 3: Independent calculations with multiple accumulators */
    /* Gives scheduler multiple independent chains to work with */
    int acc1 = 0, acc2 = 0, acc3 = 0;
    for (int i = 0; i < size; i++) {
        /* Three parallel dependency chains */
        acc1 = acc1 * 13 + a[i];
        acc2 = acc2 + b[i] * 17;
        acc3 = acc3 ^ (a[i] * b[i]);
        
        /* Cross-dependency every 8 iterations */
        if ((i & 7) == 0) {
            acc1 = acc1 ^ acc2;
            acc2 = acc2 + acc3;
        }
    }
    result3 = acc1 + acc2 - acc3;
    
    /* Loop 4: Search loop with early exit possibility */
    /* Creates control flow for scheduler to handle */
    int found_index = -1;
    int search_key = 500;  /* Mid-range value */
    for (int i = 0; i < size; i++) {
        /* Data-dependent condition */
        if (a[i] == search_key || b[i] == search_key) {
            found_index = i;
            /* Don't break immediately - let scheduler see more instructions */
            result1 += i * 2;
        }
        /* Continue computation even after finding */
        result2 += (found_index != -1) ? 1 : 0;
    }
    
    /* Combine all results */
    return result1 + result2 * 3 - result3;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int *array_b = (int*)malloc(ARRAY_SIZE * sizeof(int));
    char *array_c = (char*)malloc(ARRAY_SIZE * sizeof(char));
    short *array_d = (short*)malloc(ARRAY_SIZE * sizeof(short));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(array_a, array_b, array_c, array_d, ARRAY_SIZE);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;
    
    /* Simple validation to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    /* Print something so compiler doesn't optimize everything away */
    printf("Result: %d\n", sink);
    
    return 0;
}
