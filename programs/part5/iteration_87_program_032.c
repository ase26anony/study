/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int *arr1, int *arr2, short *arr3, char *arr4, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = (short)(lcg_rand() % 256);
        arr4[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int *arr1, int *arr2, short *arr3, char *arr4, int size) {
    int result1 = 0;
    int result2 = 0;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Tight data-dependent chain with mixed operations */
    /* Creates a long dependency chain that scheduler can pipeline */
    for (int i = 0; i < size; ++i) {
        /* Complex data-dependent chain */
        int temp = arr1[i] * 3;
        temp = (temp + arr2[i]) & 0xFFF;  /* Mask operation */
        temp = temp | (arr1[i] << 4);     /* Shift and OR */
        result1 = (result1 * 7 + temp) ^ arr2[i];  /* Chain with previous iteration */
    }
    
    /* Loop 2: Conditional operations with type promotions */
    /* char/short promotions create varied instruction mix */
    int threshold = 500;
    for (int i = 0; i < size; ++i) {
        /* Type promotions: char -> int, short -> int */
        int val1 = (int)arr4[i] * 2;      /* char promoted to int */
        int val2 = (int)arr3[i] * 3;      /* short promoted to int */
        
        /* Conditional with data-dependent operations */
        if (arr1[i] > threshold) {
            result2 += val1 * arr1[i] + val2;
        } else {
            result2 += val1 | arr2[i];
        }
        
        /* Additional operation to create scheduling choices */
        result2 = (result2 << 1) ^ (arr2[i] & 0xF);
    }
    
    /* Loop 3: Multiple independent chains within same loop */
    /* Gives scheduler multiple parallel chains to schedule */
    int chain_a = 0, chain_b = 0, chain_c = 0;
    for (int i = 0; i < size; ++i) {
        /* Chain A: Multiplicative accumulation */
        chain_a = chain_a * 13 + arr1[i];
        
        /* Chain B: Bitwise operations */
        chain_b = (chain_b ^ arr2[i]) + (arr3[i] << 2);
        
        /* Chain C: Conditional accumulation */
        if ((i & 3) == 0) {
            chain_c += arr4[i] * 5;
        } else {
            chain_c -= arr4[i];
        }
        
        /* Cross-chain dependency every 8 iterations */
        if ((i & 7) == 0) {
            chain_a ^= chain_b;
            chain_b += chain_c;
        }
    }
    result3 = chain_a + chain_b + chain_c;
    
    /* Loop 4: Search loop with early exit possibility */
    /* Creates control flow for scheduler to handle */
    int search_target = 750;
    int found_index = -1;
    for (int i = 0; i < size; ++i) {
        /* Data-dependent condition with computation */
        int test_val = arr1[i] * 2 - arr2[i];
        if (test_val > search_target && found_index < 0) {
            found_index = i;
            /* Continue computation even after finding */
            result4 += test_val * 3;
        } else {
            result4 += arr1[i] + arr2[i];
        }
        
        /* Additional operation to keep loop non-trivial */
        result4 = result4 & 0x3FFFFFFF;  /* Keep within range */
    }
    
    /* Combine all results */
    return (result1 ^ result2) + (result3 * 3) - result4;
}

int main(void) {
    const int size = 256;  /* Medium size for scheduling opportunities */
    
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(size * sizeof(int));
    int *arr2 = (int*)malloc(size * sizeof(int));
    short *arr3 = (short*)malloc(size * sizeof(short));
    char *arr4 = (char*)malloc(size * sizeof(char));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, size);
    
    /* Call work function - this is where selective scheduling happens */
    int result = work(arr1, arr2, arr3, arr4, size);
    
    /* Use result to prevent dead code elimination */
    volatile int sink = result;  /* volatile sink AFTER computation */
    
    /* Simple validation to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print minimal output to show program ran */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
