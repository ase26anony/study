/* test_sel_sched.c - Complex loop to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long compute_hash(unsigned long seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed * 1103515245 + 12345;
    unsigned long b = seed ^ 0xDEADBEEF;
    unsigned long c = 1;
    int i;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1664525 + 1013904223) % 2147483647;
        b = (b * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Bitwise operations */
        c = c ^ (a << 3);
        c = c | (b >> 5);
        
        /* Conditional operations based on computed values */
        if (a % 3 == 0) {
            c = c + (b * 7);
            /* Division creates longer latency operation */
            c = c / ((i % 5) + 1);
        } else if (a % 7 == 0) {
            c = c - (b / 3);
            c = c * 2;
        } else {
            c = c ^ (a & b);
            /* Modulo operation */
            c = c % 65536;
        }
        
        /* Nested conditional with bit manipulation */
        if ((c & 0xFF) > 128) {
            c = c << 1;
            /* Memory barrier via inline asm */
            asm volatile("" ::: "memory");
        } else {
            c = c >> 1;
        }
        
        /* Switch-like behavior */
        switch (c % 4) {
            case 0: c = c + i; break;
            case 1: c = c - (i * 2); break;
            case 2: c = c ^ (i << 8); break;
            case 3: c = c | 0xFFFF; break;
        }
        
        /* Loop-carried dependency */
        sink = c; /* Volatile write prevents elimination */
    }
    
    /* Final mixing */
    a = a ^ b ^ c;
    a = a * 0x9E3779B97F4A7C15;
    a = a ^ (a >> 30);
    
    return a;
}

/* Second function with different pattern to increase scheduling complexity */
int process_array(int *arr, int size) {
    int sum = 0;
    int prod = 1;
    int i, j;
    
    for (i = 0; i < size; i++) {
        /* Nested loop for additional complexity */
        for (j = 0; j < 3; j++) {
            arr[i] = arr[i] + (i * j);
        }
        
        /* Data-dependent branching */
        if (arr[i] > 1000) {
            sum += arr[i] / 2;
            prod *= (arr[i] % 100);
        } else if (arr[i] < 0) {
            sum -= (-arr[i]) * 3;
            prod /= ((arr[i] % 10) + 2);
        } else {
            sum += arr[i];
            prod = (prod + arr[i]) & 0xFFF;
        }
        
        /* Break condition based on computation */
        if (sum > 1000000) {
            sum = sum % 1000000;
            break;
        }
    }
    
    /* Mix results */
    return sum ^ prod;
}

/* Main driver with external input to prevent compile-time computation */
int main(int argc, char **argv) {
    unsigned long seed;
    int iterations;
    int array[100];
    int i, result1, result2;
    
    /* Get seed from command line or stdin */
    if (argc > 2) {
        seed = atol(argv[1]);
        iterations = atoi(argv[2]);
    } else {
        printf("Enter seed and iterations: ");
        scanf("%lu %d", &seed, &iterations);
    }
    
    /* Initialize array with pseudo-random values */
    for (i = 0; i < 100; i++) {
        array[i] = (i * 1103515245 + 12345) % 1000;
    }
    
    /* Call complex functions multiple times */
    result1 = compute_hash(seed, iterations);
    result2 = process_array(array, 100);
    
    /* Use results to prevent elimination */
    printf("Results: %lu %d\n", 
           (unsigned long)(result1 ^ result2), 
           result1 + result2);
    
    return 0;
}
