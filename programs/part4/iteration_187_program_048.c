/* test_sel_sched.c - Complex test case to trigger selective scheduling debug dumps */
#include <stdio.h>
#include <stdlib.h>

/* Volatile sink to prevent dead code elimination */
volatile int global_sink;

/* Complex function with data-dependent branching and mixed operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = a * 1664525 + 1013904223;
        b = b ^ (b >> 13);
        b = b ^ (b << 17);
        b = b ^ (b >> 5);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            c = c + (a & 0xFF);
            d = d ^ (c << 3);
        } else if (a % 7 == 0) {
            c = c - (b & 0xFF);
            d = d | (c >> 2);
        } else {
            c = c ^ a;
            d = d & b;
        }
        
        /* Mixed operations with different types */
        int temp = (int)(a % 100);
        if (temp < 50) {
            d = d + (unsigned int)temp;
        }
        
        /* Bitwise operations */
        c = (c << 1) | (c >> 31); /* Rotate left */
        d = (d >> 1) ^ (d << 31); /* Rotate right with XOR */
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Early exit condition */
        if (d == 0) {
            d = 1; /* Reset to avoid infinite loop */
        }
    }
    
    return a ^ b ^ c ^ d;
}

/* Another function with nested loops */
int nested_computation(int base) {
    int sum = 0;
    volatile int barrier = base; /* Volatile to prevent optimization */
    
    for (int i = 0; i < 100; i++) {
        int inner = barrier;
        for (int j = 0; j < 50; j++) {
            /* Complex expression with multiple operations */
            inner = (inner * 1103515245 + 12345) & 0x7fffffff;
            inner = (inner >> 16) & 0x7FF;
            
            /* Switch statement for control flow complexity */
            switch (inner % 5) {
                case 0: sum += inner * 2; break;
                case 1: sum -= inner / 2; break;
                case 2: sum ^= inner; break;
                case 3: sum |= inner << 3; break;
                case 4: sum &= ~inner; break;
            }
            
            /* Division/modulo operations (expensive) */
            if (j % 7 == 0) {
                sum = sum / (inner % 10 + 1);
            }
        }
        
        /* Conditional break */
        if (sum > 1000000) {
            sum = sum % 1000000;
            break;
        }
    }
    
    return sum;
}

/* Main driver with input-dependent behavior */
int main(int argc, char *argv[]) {
    unsigned int seed = 42;
    int iterations = 1000;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 10000) iterations = 10000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = 0;
    int result2 = 0;
    
    for (int run = 0; run < 3; run++) {
        result1 ^= complex_loop(seed + run, iterations);
        result2 += nested_computation(seed + run);
        
        /* Feed results into volatile sink */
        global_sink = result1 + result2;
    }
    
    /* Final output to prevent elimination */
    printf("Result: %u %d\n", result1, result2);
    
    return 0;
}
