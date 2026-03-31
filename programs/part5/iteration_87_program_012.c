/* sel-sched-trigger.c
 * Designed to trigger selective scheduler debug dumps in sel-sched-dump.cc
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 256

/* Simple deterministic pseudo-random generator */
static inline int simple_rand(int *seed) {
    *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;
    return *seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *a, int *b, short *c, char *d, int size) {
    int seed = 42;
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand(&seed) % 100;
        b[i] = simple_rand(&seed) % 100;
        c[i] = simple_rand(&seed) % 100;
        d[i] = simple_rand(&seed) % 100;
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *a, int *b, short *c, char *d, int size) {
    int result1 = 0, result2 = 0, result3 = 0;
    int temp1, temp2;
    short temp_short;
    char temp_char;
    
    /* Loop 1: Tight loop with data-dependent chain and mixed operations */
    /* sum = (sum * a[i]) + b[i] pattern */
    result1 = 1;
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * a[i]) + b[i];
    }
    
    /* Loop 2: Loop with condition and mixed data types */
    /* Uses short and char types with promotion */
    result2 = 0;
    for (int i = 0; i < size; i++) {
        temp_short = c[i];
        temp_char = d[i];
        if (temp_short > 50) {
            result2 += temp_short * temp_char;  /* Promotion happens here */
        }
    }
    
    /* Loop 3: Independent parallel chains with bitwise operations */
    /* Two independent dependency chains in same loop */
    temp1 = 0x5A5A5A5A;
    temp2 = 0x33333333;
    for (int i = 0; i < size; ++i) {
        /* First chain: bitwise operations */
        temp1 = (temp1 ^ a[i]) | b[i];
        
        /* Second chain: arithmetic with shift */
        temp2 = (temp2 + c[i]) << 1;
    }
    result3 = temp1 ^ temp2;
    
    /* Loop 4: Search loop with early exit possibility */
    /* Creates control flow for scheduler */
    int found_index = -1;
    int threshold = 75;
    for (int i = 0; i < size; i++) {
        if (a[i] > threshold && b[i] > threshold) {
            found_index = i;
            break;
        }
    }
    
    /* Loop 5: Reduction with multiple accumulators */
    /* Gives scheduler multiple independent ops to schedule */
    int sum1 = 0, sum2 = 0, sum3 = 0;
    for (int i = 0; i < size; i += 4) {
        sum1 += a[i];
        sum2 += b[i+1];
        sum3 += c[i+2] + d[i+3];
    }
    int result4 = sum1 + sum2 + sum3;
    
    /* Combine all results */
    return result1 + result2 + result3 + result4 + found_index;
}

int main(void) {
    /* Allocate and initialize arrays */
    int *array_a = (int*)malloc(SIZE * sizeof(int));
    int *array_b = (int*)malloc(SIZE * sizeof(int));
    short *array_c = (short*)malloc(SIZE * sizeof(short));
    char *array_d = (char*)malloc(SIZE * sizeof(char));
    
    if (!array_a || !array_b || !array_c || !array_d) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array_a, array_b, array_c, array_d, SIZE);
    
    /* Call work function - this should trigger selective scheduling */
    int final_result = work(array_a, array_b, array_c, array_d, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Simple validation to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(array_a);
    free(array_b);
    free(array_c);
    free(array_d);
    
    return 0;
}
