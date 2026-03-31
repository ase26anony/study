/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio constant */
    unsigned int c = 0;
    unsigned int d = 1;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 1664525 + 1013904223;
        b = b ^ (b >> 13);
        b = b ^ (b << 17);
        b = b ^ (b >> 5);
        
        /* Conditional operations based on computed values */
        if ((a % 7) == 0) {
            c = c + (a & 0xFF);
            d = d * 3;
        } else if ((a % 13) == 0) {
            c = c ^ (b & 0xFF);
            d = d / 2;
        } else {
            c = c | (a ^ b);
            d = d % 1023;
        }
        
        /* Bitwise operations with shifts */
        c = (c << 3) | (c >> 29); /* Rotate left 3 */
        d = (d << 1) + (d >> 31); /* Rotate right 1 */
        
        /* Memory barrier to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Nested conditional with loop-carried dependency */
        if (c > 0x80000000) {
            for (int j = 0; j < 2; j++) {
                d = d + (c & 0xF);
                c = c >> 4;
            }
        }
        
        /* Switch statement for additional control flow complexity */
        switch (i % 5) {
            case 0: a = a + b; break;
            case 1: a = a - c; break;
            case 2: a = a * d; break;
            case 3: a = a ^ d; break;
            case 4: a = a | c; break;
        }
    }
    
    /* Final computation with volatile write */
    unsigned int result = (a ^ b) + (c * d);
    sink = result; /* Volatile write prevents elimination */
    
    return result;
}

/* Another function with different pattern to increase scheduling complexity */
int nested_loops(int start, int limit) {
    int sum = 0;
    volatile int vsink;
    
    for (int i = start; i < limit; i++) {
        int temp = i;
        
        /* Inner loop with break condition */
        for (int j = 0; j < 8; j++) {
            if (temp <= 0) break;
            
            /* Mixed operations creating dependencies */
            temp = (temp * 1103515245 + 12345) & 0x7FFFFFFF;
            sum += (temp % 19) - 9;
            
            /* More arithmetic */
            sum = sum ^ (temp << (j % 16));
            sum = abs(sum); /* Function call adds complexity */
        }
        
        /* Conditional with division (expensive operation) */
        if (i % 11 == 0) {
            sum = sum / 2;
        } else {
            sum = sum * 3 + 1;
        }
    }
    
    vsink = sum;
    return sum;
}

/* Main function with external input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    int iterations = 100;
    
    /* Use command line arguments to prevent constant propagation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = complex_loop(seed, iterations);
    int result2 = nested_loops(1, 50);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %u, %d\n", result1, result2);
    
    /* Additional calls with different parameters */
    result1 = complex_loop(result1, iterations / 2);
    result2 = nested_loops(result2 % 100, 30);
    
    printf("Final: %u, %d\n", result1, result2);
    
    return 0;
}
