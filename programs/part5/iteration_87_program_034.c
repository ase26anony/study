/* sel-sched-trigger.c
 * Program designed to trigger selective scheduler debug dumps
 * Compile with: gcc -O2 -fsel-sched-pipelining -fdump-rtl-all -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int simple_rand(unsigned int seed) {
    return seed * 1103515245u + 12345u;
}

/* Fill arrays with deterministic pseudo-random values */
static void init_arrays(int* arr1, int* arr2, char* arr3, short* arr4, int size, unsigned int seed) {
    unsigned int r = seed;
    for (int i = 0; i < size; i++) {
        r = simple_rand(r);
        arr1[i] = (int)(r % 1000);
        arr2[i] = (int)(r % 500);
        arr3[i] = (char)(r % 128);
        r = simple_rand(r);
        arr4[i] = (short)(r % 10000);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int* arr1, int* arr2, char* arr3, short* arr4, int size) {
    int result1 = 0;
    int result2 = 1;
    int result3 = 0;
    int result4 = 0;
    
    /* Loop 1: Multiplicative-accumulative with loop-carried dependency */
    /* sum = (sum * a) + b pattern */
    for (int i = 0; i < size; ++i) {
        result1 = (result1 * arr1[i]) + arr2[i];
    }
    
    /* Loop 2: Conditional accumulation with mixed types */
    /* Uses char array with promotion to int */
    int threshold = 50;
    int scale = 3;
    for (int i = 0; i < size; ++i) {
        if (arr3[i] > threshold) {
            result2 += (int)arr3[i] * scale;
        } else {
            result2 -= (int)arr3[i];
        }
    }
    
    /* Loop 3: Bitwise operations with short type */
    /* Multiple uses of same variable in different operations */
    unsigned int mask = 0x00FF00FF;
    for (int i = 0; i < size; ++i) {
        unsigned int val = (unsigned int)arr4[i];
        result3 = (result3 & mask) | (val << 8);
        result3 = result3 ^ (val + i);
    }
    
    /* Loop 4: Complex dependency chain with multiple operations */
    /* a = (a + b) * c; b = a ^ d; pattern */
    int a = result1 % 100;
    int b = result2 % 100;
    for (int i = 0; i < size; ++i) {
        a = (a + arr1[i]) * (arr2[i] % 10 + 1);
        b = a ^ arr4[i];
        result4 += a + b;
    }
    
    /* Combine all results */
    return result1 ^ result2 ^ result3 ^ result4;
}

int main() {
    const int SIZE = 256;
    unsigned int seed = 42;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    char* arr3 = (char*)malloc(SIZE * sizeof(char));
    short* arr4 = (short*)malloc(SIZE * sizeof(short));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, SIZE, seed);
    
    /* Call work function with multiple scheduling opportunities */
    int final_result = work(arr1, arr2, arr3, arr4, SIZE);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile int sink = final_result;
    
    /* Simple validation to ensure code isn't removed */
    if (sink == 0xDEADBEEF) {
        __builtin_trap();  /* This should never happen */
    }
    
    /* Print result to prevent optimization */
    printf("Result: %d\n", sink);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
