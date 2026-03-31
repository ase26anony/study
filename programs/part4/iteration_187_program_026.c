/* test_sel_sched.c - Complex loop to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long compute_hash(unsigned long seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245 + 12345;
    unsigned long b = seed ^ 0xDEADBEEF;
    unsigned long c = 1;
    unsigned long result = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex data-dependent branching */
        if (i % 3 == 0) {
            a = (a * 1664525 + 1013904223) % 2147483647;
            b = b ^ (a << 3);
            c = c + (a >> 5);
        } else if (i % 3 == 1) {
            a = a + (b * 1103515245);
            b = (b << 7) | (b >> 25);
            c = c ^ (a & 0xFFFFFFFF);
        } else {
            a = (a ^ b) + c;
            b = b * 6364136223846793005UL;
            c = (c << 13) | (c >> 51);
        }
        
        /* More arithmetic operations */
        result += (a % 65537) * (b % 65537);
        result ^= (c & 0xFFFF);
        
        /* Nested conditional with break */
        if (result > 0x7FFFFFFF && i > iterations/2) {
            result = result % 1000000007;
            if (result < 1000) break;
        }
        
        /* Bitwise operations */
        result = (result << 3) | (result >> 61);
        result = result ^ (i * 0x9E3779B9);
    }
    
    /* Memory barrier and volatile write */
    asm volatile("" ::: "memory");
    sink = result;
    
    return result + a + b + c;
}

/* Another function with different pattern */
int process_array(int *arr, int n) {
    volatile int vsink;
    int sum = 0;
    int prod = 1;
    
    for (int i = 0; i < n; i++) {
        /* Loop-carried dependency */
        sum += arr[i];
        
        /* Conditional with multiple operations */
        if (arr[i] % 2 == 0) {
            prod *= (arr[i] + 1);
            sum = sum ^ (prod & 0xFF);
        } else {
            prod = prod / 2 + arr[i];
            sum = sum - (arr[i] % 256);
        }
        
        /* More complex branching */
        switch (i % 4) {
            case 0: sum = sum << 1; break;
            case 1: sum = sum >> 1; break;
            case 2: sum = sum ^ 0xAAAA; break;
            case 3: sum = sum + 0x5555; break;
        }
        
        /* Prevent loop unrolling */
        if (i == n-1 && sum > 1000000) {
            sum = sum % 10007;
        }
    }
    
    vsink = sum;
    asm volatile("" ::: "memory");
    
    return sum * prod;
}

/* Main driver with external input */
int main(int argc, char **argv) {
    int iterations = 1000;
    int array_size = 500;
    
    /* Use command line input to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 100) iterations = 100;
        if (iterations > 10000) iterations = 10000;
    }
    
    if (argc > 2) {
        array_size = atoi(argv[2]);
        if (array_size < 50) array_size = 50;
        if (array_size > 1000) array_size = 1000;
    }
    
    /* Initialize array with pseudo-random values */
    int *array = (int*)malloc(array_size * sizeof(int));
    unsigned long seed = 123456789;
    
    for (int i = 0; i < array_size; i++) {
        seed = seed * 1103515245 + 12345;
        array[i] = (int)(seed % 1000);
    }
    
    /* Call both complex functions */
    unsigned long hash_result = compute_hash(seed, iterations);
    int array_result = process_array(array, array_size);
    
    /* Combine results to prevent elimination */
    unsigned long final_result = hash_result + array_result;
    
    printf("Result: %lu\n", final_result % 1000000007);
    
    free(array);
    return 0;
}
