/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long compute_hash(unsigned long seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0x9e3779b97f4a7c15UL;
    unsigned long c = 0xdeadbeefcafebabeUL;
    int i;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 6364136223846793005UL + 1442695040888963407UL;
        b = b ^ (b >> 27);
        c = c * 2862933555777941757UL + 3037000493UL;
        
        /* Control flow based on computed values */
        if ((a & 0xFF) == 0) {
            /* Bitwise operations */
            b = (b << 13) | (b >> 51);
            c = c + (a & 0xFFFF);
        } else if ((b % 7) == 0) {
            /* Different arithmetic */
            c = c - (a % 256);
            b = b ^ c;
        } else {
            /* More operations */
            a = a ^ b ^ c;
            c = c * 3 + 1;
        }
        
        /* Nested conditional with modulo */
        switch (i % 5) {
            case 0: a = a + (b << 2); break;
            case 1: b = b - (c >> 3); break;
            case 2: c = c ^ (a * b); break;
            case 3: a = (a & b) | c; break;
            case 4: b = (b % 997) + c; break;
        }
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
    }
    
    /* Final mixing */
    a = a ^ (a >> 33);
    a = a * 0xff51afd7ed558ccdUL;
    a = a ^ (a >> 33);
    a = a * 0xc4ceb9fe1a85ec53UL;
    a = a ^ (a >> 33);
    
    sink = a; /* Volatile write to prevent elimination */
    return a ^ b ^ c;
}

/* Another function with different pattern */
int process_array(int *arr, int n) {
    int sum = 0;
    int prod = 1;
    int i, j;
    
    /* Outer loop with inner conditional */
    for (i = 0; i < n; i++) {
        /* Loop-carried dependency */
        sum += arr[i];
        
        /* Inner loop with break condition */
        for (j = 0; j < 3; j++) {
            if ((arr[i] & (1 << j)) == 0) {
                prod *= (i + j + 1);
            } else {
                prod /= (j + 1);
                if (prod == 0) break;
            }
        }
        
        /* More arithmetic with different types */
        long temp = (long)sum * (long)prod;
        arr[i] = (int)(temp % 1000000);
        
        /* Conditional with complex expression */
        if ((sum ^ prod) > 1000 && i % 4 == 0) {
            arr[i] = ~arr[i];
        }
    }
    
    /* Volatile sink */
    volatile int vsink = sum;
    return prod;
}

/* Main driver with external input */
int main(int argc, char **argv) {
    int iterations = 100;
    int array_size = 50;
    unsigned long hash_result;
    int array_result;
    
    /* Use command line argument to prevent constant folding */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 100;
    }
    
    /* Initialize array with non-constant values */
    int *data = malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        data[i] = (i * 37 + 123) % 7919;
    }
    
    /* Call both complex functions */
    hash_result = compute_hash(0x123456789ABCDEFUL, iterations);
    array_result = process_array(data, array_size);
    
    /* Use results to prevent elimination */
    printf("Hash: 0x%016lX\n", hash_result);
    printf("Array result: %d\n", array_result);
    printf("Data[0]: %d\n", data[0]);
    
    free(data);
    return (int)(hash_result & 0x7FFFFFFF);
}
