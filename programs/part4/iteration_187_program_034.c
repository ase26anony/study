/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0xDEADBEEF;
    unsigned long c = 0;
    unsigned long d = 1;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1103515245 + 12345) & 0x7FFFFFFF;
        b = b ^ (a << 13) ^ (a >> 17);
        c = c + (a % 997) - (b % 991);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            d = d * 3 + 1;
            /* Bitwise operations */
            c = c ^ (d << 5);
        } else if (a % 7 == 0) {
            d = d / 2;
            /* More arithmetic */
            b = b + d * 7;
        } else {
            d = d + a;
            /* Mixed operations */
            c = (c << 1) | (c >> 31);
        }
        
        /* Nested conditional with different operations */
        switch (i % 5) {
            case 0: a = a + b; break;
            case 1: b = b - c; break;
            case 2: c = c * 2; break;
            case 3: d = d ^ a; break;
            case 4: a = a | b; break;
        }
        
        /* Artificial memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Combine results to create final output */
    unsigned long result = a ^ b ^ c ^ d;
    sink = result; /* Volatile write to prevent elimination */
    
    return result;
}

/* Another function with different pattern */
int nested_loops(int start, int limit) {
    int sum = 0;
    volatile int barrier;
    
    for (int i = start; i < limit; i++) {
        int inner_sum = 0;
        
        /* Inner loop with break condition */
        for (int j = 0; j < 10; j++) {
            if (j > i % 8) break;
            
            inner_sum += (i * j) % 256;
            inner_sum = inner_sum ^ (inner_sum >> 3);
            
            /* More operations to increase instruction count */
            inner_sum = (inner_sum * 13) % 10007;
        }
        
        /* Conditional update */
        if (inner_sum % 2 == 0) {
            sum += inner_sum * 3;
        } else {
            sum -= inner_sum / 2;
        }
        
        /* Complex expression */
        sum = (sum << 4) | (sum >> 28);
    }
    
    barrier = sum;
    return sum;
}

/* Function with pointer arithmetic and memory access */
void process_array(int *arr, int size) {
    volatile int temp;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent array access */
        int idx = (i * 31) % size;
        
        /* Multiple operations on array elements */
        arr[idx] = arr[idx] * 2 + 1;
        arr[idx] = arr[idx] ^ (arr[idx] >> 16);
        
        /* Conditional with division (expensive operation) */
        if (arr[idx] % 11 == 0) {
            arr[idx] = arr[idx] / 3;
        } else {
            arr[idx] = arr[idx] % 1000;
        }
        
        /* Memory barrier */
        asm volatile("" ::: "memory");
    }
    
    temp = arr[0];
}

int main(int argc, char **argv) {
    int iterations = 100;
    
    /* Use command line argument to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    
    for (int i = 0; i < 5; i++) {
        total += complex_loop(i * 12345, iterations);
        total = (total << 3) | (total >> 61);
    }
    
    int array_sum = nested_loops(1, iterations / 2);
    total += array_sum;
    
    /* Process an array */
    int arr[100];
    for (int i = 0; i < 100; i++) {
        arr[i] = i * 3 + 1;
    }
    process_array(arr, 100);
    
    /* Use array result */
    for (int i = 0; i < 10; i++) {
        total += arr[i * 7 % 100];
    }
    
    printf("Result: %lu\n", total);
    return (int)(total % 256);
}
