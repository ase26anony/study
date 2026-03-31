/* test_sel_sched.c - Complex loop to trigger selective scheduling debug output */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long compute_hash(unsigned long seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245 + 12345;
    unsigned long b = seed ^ 0xDEADBEEF;
    unsigned long c = 1;
    unsigned long result = 0;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1664525 + 1013904223) % 0xFFFFFFFF;
        b = (b ^ (b >> 17)) * 0x9E3779B9;
        c = c + (a & 0xFF) - (b & 0xFF);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            result += (a << 2) | (b >> 3);
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        } else if (a % 7 == 0) {
            result ^= (b * c) + (a / 7);
            c = (c << 1) | (c >> 31);
        } else {
            result = (result * 13) + (a % 19);
            b = b ^ c ^ a;
        }
        
        /* More mixed operations */
        if (i % 5 == 0) {
            result = result - (c & 0xFFFF);
            a = a ^ result;
        }
        
        /* Nested conditional with bitwise operations */
        switch (i % 4) {
            case 0: result = result | 0xAAAAAAAA; break;
            case 1: result = result & 0x55555555; break;
            case 2: result = result ^ 0x33333333; break;
            case 3: result = result + (result << 8); break;
        }
        
        /* Loop-carried dependency */
        c = c + result;
    }
    
    /* Final mixing */
    result = result ^ (result >> 16);
    result = result * 0x85EBCA6B;
    result = result ^ (result >> 13);
    result = result * 0xC2B2AE35;
    result = result ^ (result >> 16);
    
    sink = result; /* Volatile write to prevent elimination */
    return result;
}

/* Another function with different pattern */
int process_array(int *arr, int size) {
    volatile int checksum = 0;
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < size; i++) {
        /* Complex data flow */
        int val = arr[i];
        
        if (val > 0) {
            sum += val * 2;
            prod *= (val % 10) + 1;
            /* Conditional with division (expensive) */
            if (val % 11 == 0) {
                sum /= 2;
            }
        } else if (val < 0) {
            sum -= (-val) << 1;
            prod /= 2;
        } else {
            sum ^= i;
            prod |= 0xFF;
        }
        
        /* More operations to increase instruction count */
        for (int j = 0; j < 3; j++) {
            val = (val * 31 + 17) % 256;
            checksum += val;
        }
        
        /* Break condition based on computed value */
        if (checksum > 10000) {
            checksum = checksum % 1000;
            break;
        }
    }
    
    return sum ^ prod;
}

int main(int argc, char **argv) {
    int iterations = 1000;
    int array_size = 500;
    
    /* Use command line argument to prevent constant folding */
    if (argc > 1) {
        iterations = atoi(argv[1]) % 1000 + 100;
        array_size = atoi(argv[2]) % 400 + 100;
    }
    
    /* Initialize array with pseudo-random values */
    int *array = malloc(array_size * sizeof(int));
    unsigned long seed = 42;
    
    for (int i = 0; i < array_size; i++) {
        seed = seed * 1103515245 + 12345;
        array[i] = (int)(seed % 1000) - 500;
    }
    
    /* Call complex functions multiple times */
    unsigned long hash1 = compute_hash(1, iterations);
    unsigned long hash2 = compute_hash(hash1, iterations / 2);
    int array_result = process_array(array, array_size);
    
    /* Mix results to produce final output */
    unsigned long final_result = hash2 ^ (array_result * 0x100000001UL);
    
    printf("Result: %lu\n", final_result % 1000000);
    
    free(array);
    return 0;
}
