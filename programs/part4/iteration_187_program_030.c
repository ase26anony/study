/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
    for (int i = 0; i < iterations; i++) {
        /* Mix operations with data-dependent control flow */
        if (i % 3 == 0) {
            a = (a ^ (a >> 13)) * 0x85ebca6b;
            b = (b + (b << 7)) | 0x3a7d8a9f;
        } else if (i % 3 == 1) {
            a = (a * 0xcc9e2d51) + 0x1b873593;
            b = b ^ (b >> 15);
        } else {
            a = a + (a << 16);
            b = b - (b << 3);
        }
        
        /* More arithmetic with different data types */
        c = c ^ a;
        d = d + b;
        
        /* Nested conditional with bitwise operations */
        if ((c & 0xff) > 128) {
            d = d ^ (c << 1);
            /* Inline asm to create memory barrier */
            __asm__ volatile ("" ::: "memory");
        } else {
            c = c | (d >> 2);
        }
        
        /* Loop-carried dependency */
        a = a + c;
        b = b - d;
        
        /* Modulo operation creates complex dependency */
        if (i % 7 == 0) {
            unsigned int temp = a % 17;
            b = b ^ temp;
            c = c + temp;
        }
        
        /* Switch-like structure */
        switch (d & 0x3) {
            case 0: a = a << 1; break;
            case 1: a = a >> 1; break;
            case 2: a = a ^ 0xaaaaaaaa; break;
            case 3: a = a + 0x55555555; break;
        }
    }
    
    /* Final mixing */
    a = a ^ (a >> 16);
    b = b ^ (b << 13);
    c = c ^ (c >> 17);
    d = d ^ (d << 5);
    
    sink = a + b + c + d; /* Volatile write to prevent elimination */
    return a ^ b ^ c ^ d;
}

/* Another function with different pattern */
int nested_loops(int start, int limit) {
    int sum = 0;
    volatile int vsink;
    
    for (int i = start; i < limit; i++) {
        int inner_sum = 0;
        for (int j = 0; j < 10; j++) {
            /* Break condition based on computation */
            if ((i * j) % 11 == 0) {
                inner_sum += i / (j + 1);
            } else {
                inner_sum += i % (j + 2);
            }
            
            /* More operations */
            inner_sum = inner_sum ^ (i << j);
            inner_sum = inner_sum * 1103515245 + 12345;
        }
        
        /* Conditional break in outer loop */
        if (inner_sum > 1000000) {
            sum += inner_sum % 1000;
            break;
        }
        sum += inner_sum;
    }
    
    vsink = sum;
    return sum;
}

/* Main driver with input-dependent execution */
int main(int argc, char *argv[]) {
    unsigned int result1, result2;
    int result3;
    
    /* Use command line or stdin for input variability */
    unsigned int seed = 12345;
    int iterations = 100;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    result1 = complex_loop(seed, iterations);
    result2 = complex_loop(result1, iterations / 2);
    result3 = nested_loops(seed % 100, iterations % 50 + 20);
    
    /* Combine results to produce final output */
    unsigned int final_result = result1 ^ result2 ^ (unsigned int)result3;
    printf("Result: %u\n", final_result);
    
    return (int)(final_result % 256);
}
