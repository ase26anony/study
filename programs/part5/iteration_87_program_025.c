/* Selective scheduling trigger program for GCC sel-sched-dump.cc coverage */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int lcg_seed = 123456789;
static inline unsigned int lcg_rand() {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Initialize arrays with deterministic values */
static void init_arrays(int* arr1, int* arr2, short* arr3, char* arr4, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = (short)(lcg_rand() % 256);
        arr4[i] = (char)(lcg_rand() % 128);
    }
}

/* Work function with multiple loops for selective scheduling */
__attribute__((noinline))
static int work(int* data1, int* data2, short* data3, char* data4, int n) {
    int result = 0;
    int temp1 = 1, temp2 = 0;
    int threshold = 500;
    char scale = 3;
    
    /* Loop 1: Multiplicative-accumulate with data-dependent chain */
    for (int i = 0; i < n; ++i) {
        /* Creates dependency chain: temp1 → temp1 * data1[i] → result */
        temp1 = (temp1 * data1[i]) + data2[i];
        result ^= temp1;  /* Use XOR to prevent simple reduction */
    }
    
    /* Loop 2: Conditional accumulation with mixed types */
    for (int i = 0; i < n; ++i) {
        /* Mix int and short types, condition creates control flow */
        if (data1[i] > threshold) {
            /* Type promotion: short → int, char → int */
            int val = (int)data3[i] * (int)scale;
            result += val;
            temp2 |= val;  /* Additional dependency */
        } else {
            result -= data2[i] & 0xFF;  /* Different operation */
        }
    }
    
    /* Loop 3: Bit manipulation loop with carry chain */
    unsigned int bits = 0x5A5A5A5A;
    for (int i = 0; i < n; ++i) {
        /* Multiple dependent operations in sequence */
        bits = (bits << 1) | ((unsigned int)data4[i] & 1);
        bits ^= (unsigned int)data1[i];
        bits = (bits >> 3) ^ (bits << 5);
        result += (int)(bits & 0xFFFF);
    }
    
    /* Loop 4: Search with early exit possibility */
    int found = 0;
    for (int i = 0; i < n && !found; ++i) {
        /* Data-dependent condition with arithmetic */
        int test = data1[i] * 2 - data2[i];
        if (test > 750) {
            result += i * 100;
            found = 1;  /* Potentially breaks dependency chain */
        }
        /* Always execute this part */
        result += data3[i] % 16;
    }
    
    /* Combine all intermediate results */
    return result + temp1 + temp2;
}

int main(int argc, char** argv) {
    /* Use command line or fixed size to prevent constant propagation */
    int size = (argc > 1) ? atoi(argv[1]) : 200;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(size * sizeof(int));
    int* arr2 = (int*)malloc(size * sizeof(int));
    short* arr3 = (short*)malloc(size * sizeof(short));
    char* arr4 = (char*)malloc(size * sizeof(char));
    
    if (!arr1 || !arr2 || !arr3 || !arr4) {
        return 1;
    }
    
    init_arrays(arr1, arr2, arr3, arr4, size);
    
    /* Call work function with all loops */
    int final_result = work(arr1, arr2, arr3, arr4, size);
    
    /* Prevent dead code elimination without affecting scheduling */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    /* Print to prevent optimization */
    printf("Result: %d\n", final_result);
    
    return 0;
}
