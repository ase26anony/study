/* Selective scheduling test program targeting sel-sched-dump.cc debug output */
#include <stdio.h>
#include <stdlib.h>

/* Simple deterministic pseudo-random generator */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
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
static int work(int *data1, int *data2, short *data3, char *data4, int n) {
    int result = 0;
    int temp1 = 1, temp2 = 0;
    short temp_short = 100;
    char temp_char = 50;
    
    /* Loop 1: Multiplicative-accumulative with data-dependent chain */
    for (int i = 0; i < n; ++i) {
        temp1 = (temp1 * data1[i]) + data2[i];
        result ^= temp1;  /* Use result to create cross-loop dependency */
    }
    
    /* Loop 2: Mixed-type operations with condition */
    for (int i = 0; i < n; ++i) {
        if (data3[i] > temp_short) {
            temp2 += (int)data3[i] * (int)data4[i];
        } else {
            temp2 -= (int)data3[i] | (int)data4[i];
        }
        /* Create short dependency chain within loop */
        temp_short = (temp_short + data4[i]) & 0xFF;
    }
    result += temp2;
    
    /* Loop 3: Bit manipulation with loop-carried dependency */
    int bit_acc = 0x5A5A5A5A;
    for (int i = 0; i < n; ++i) {
        bit_acc = (bit_acc ^ data1[i]) & (bit_acc | data2[i]);
        bit_acc = (bit_acc << 3) | (bit_acc >> 29);  /* Rotation */
        temp_char = (temp_char ^ data4[i]) + 1;
    }
    result ^= bit_acc;
    
    /* Loop 4: Short chain with multiple uses of same variable */
    int chain = result;
    for (int i = 0; i < n; ++i) {
        chain = chain + data1[i];
        chain = chain * 3;
        chain = chain - data2[i];
        chain = chain & 0xFFFF;
    }
    result = chain;
    
    return result;
}

int main(void) {
    const int size = 256;
    int *array1 = (int*)malloc(size * sizeof(int));
    int *array2 = (int*)malloc(size * sizeof(int));
    short *array3 = (short*)malloc(size * sizeof(short));
    char *array4 = (char*)malloc(size * sizeof(char));
    
    if (!array1 || !array2 || !array3 || !array4) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, array4, size);
    
    /* Call work function - this should trigger selective scheduling */
    int final_result = work(array1, array2, array3, array4, size);
    
    /* Prevent dead code elimination without preventing scheduling */
    volatile int sink = final_result;
    
    /* Simple side effect to ensure code isn't removed */
    if (sink == 0x12345678) {  /* Unlikely value */
        __builtin_trap();
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", sink);
    
    free(array1);
    free(array2);
    free(array3);
    free(array4);
    
    return 0;
}
