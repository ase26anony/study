/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long compute_hash(unsigned long seed, int iterations) {
    volatile unsigned long state = seed; /* Prevent optimization */
    unsigned long a = 0x9e3779b9;
    unsigned long b = 0x6a09e667;
    unsigned long c = 0xbb67ae85;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        state = state * 1664525UL + 1013904223UL;
        
        /* Data-dependent branching */
        if (state & 0x1) {
            a = a ^ (state >> 1);
            b = b + (a * 3);
        } else {
            a = a + (state >> 2);
            b = b ^ (a << 1);
        }
        
        /* Mixed bitwise and arithmetic operations */
        c = (c * 13) ^ (state & 0xFF);
        
        /* More complex conditional */
        switch (state % 7) {
            case 0: a = (a << 3) | (a >> 29); break;
            case 1: b = (b * 17) + 1; break;
            case 2: c = c ^ (a & b); break;
            case 3: a = a - b + c; break;
            case 4: b = (b % 1023) ^ c; break;
            case 5: c = (c * c) / 3; break;
            default: state = state ^ (a + b + c); break;
        }
        
        /* Nested loop with break condition */
        for (int j = 0; j < 3; j++) {
            if ((state & (1 << j)) == 0) {
                a = a + j;
                if (j == 2) break;
            }
            b = b - j;
        }
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
    }
    
    /* Final mixing */
    a = a ^ (a >> 16);
    b = b ^ (b << 13);
    c = c ^ (c >> 7);
    
    return a ^ b ^ c;
}

/* Another function with different pattern */
int process_array(int *arr, int size) {
    volatile int sum = 0;
    int prod = 1;
    int xor_val = 0;
    
    for (int i = 0; i < size; i++) {
        /* Loop-carried dependency */
        sum = sum + arr[i];
        
        /* Complex conditional chain */
        if (arr[i] > 0) {
            prod = prod * (arr[i] % 256 + 1);
            if (prod > 1000000) prod = prod / 2;
        } else if (arr[i] < 0) {
            xor_val = xor_val ^ (-arr[i]);
        } else {
            /* Division creates longer latency operations */
            prod = prod / 2;
        }
        
        /* More operations with different data types */
        long temp = (long)sum * (long)prod;
        xor_val = xor_val ^ (temp & 0xFFFFFFFF);
        
        /* Another asm barrier */
        asm volatile("" ::: "memory");
    }
    
    return sum + prod + xor_val;
}

/* Main driver with external input to prevent compile-time computation */
int main(int argc, char **argv) {
    int iterations = 100;
    int array_size = 50;
    
    /* Use command line or default */
    if (argc > 1) iterations = atoi(argv[1]);
    if (argc > 2) array_size = atoi(argv[2]);
    
    /* Initialize array with pseudo-random values */
    int *array = malloc(array_size * sizeof(int));
    if (!array) return 1;
    
    unsigned long seed = 123456789;
    for (int i = 0; i < array_size; i++) {
        seed = seed * 1103515245 + 12345;
        array[i] = (int)(seed % 1000) - 500;
    }
    
    /* Call both complex functions */
    unsigned long hash = compute_hash(seed, iterations);
    int array_result = process_array(array, array_size);
    
    /* Use results to prevent dead code elimination */
    printf("Hash: %lu, Array result: %d\n", hash, array_result);
    
    free(array);
    return 0;
}
