/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned long seed, int iterations) {
    volatile unsigned long sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0x9e3779b97f4a7c15UL;
    unsigned long c = 0;
    unsigned long d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 30)) * 0xbf58476d1ce4e5b9UL;
        a = (a ^ (a >> 27)) * 0x94d049bb133111ebUL;
        a = a ^ (a >> 31);
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            b = (b * 1103515245UL + 12345) & 0x7fffffffUL;
            c += (b >> 16) & 0xFF;
        } else if (a % 5 == 0) {
            b = b ^ (b << 13);
            b = b ^ (b >> 17);
            b = b ^ (b << 5);
            c -= (b >> 24) & 0xFF;
        } else {
            b = (b + a) | 1;
            c ^= b;
        }
        
        /* Mixed operations with different data types */
        d = d * 6364136223846793005UL + 1442695040888963407UL;
        int temp = (int)(d & 0xFF);
        
        /* Bitwise operations */
        c = (c << temp) | (c >> (64 - temp));
        c = c ^ d;
        
        /* Memory barrier via inline asm */
        asm volatile("" ::: "memory");
        
        /* Conditional break based on computed value */
        if ((c & 0xFFFFF) == 0 && i > iterations/2) {
            break;
        }
    }
    
    /* Final computation with division (expensive) */
    unsigned long result = (a / (b | 1)) + (c % (d | 1));
    
    /* Volatile write to prevent elimination */
    sink = result;
    return result + sink;
}

/* Another function with nested loops */
int matrix_operation(int size) {
    volatile int sink;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Complex addressing pattern */
            int idx = (i * 17 + j * 13) % size;
            
            /* Mixed operations */
            int val = (i * j + idx) ^ (i - j);
            val = (val * 1103515245 + 12345) & 0x7fffffff;
            
            /* Conditional based on multiple factors */
            if ((i + j) % 2 == 0) {
                val = val >> 4;
            } else {
                val = val << 2;
            }
            
            /* Modulo operation */
            sum += val % 256;
            
            /* Early exit condition */
            if (sum > 1000000) {
                goto done;
            }
        }
        
        /* Switch statement for control flow variety */
        switch (i % 4) {
            case 0: sum ^= 0xAA; break;
            case 1: sum |= 0x55; break;
            case 2: sum &= 0xF0; break;
            case 3: sum = ~sum; break;
        }
    }
    
done:
    sink = sum;
    return sum ^ sink;
}

/* Main driver with external input */
int main(int argc, char *argv[]) {
    unsigned long seed = 0x123456789ABCDEFUL;
    int iterations = 1000;
    int size = 50;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = strtoul(argv[1], NULL, 0);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 10) iterations = 10;
        if (iterations > 10000) iterations = 10000;
    }
    if (argc > 3) {
        size = atoi(argv[3]);
        if (size < 10) size = 10;
        if (size > 200) size = 200;
    }
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    for (int i = 0; i < 3; i++) {
        total += complex_loop(seed + i, iterations);
        total ^= matrix_operation(size + i);
    }
    
    /* Use result to prevent elimination */
    printf("Result: %lu\n", total);
    return (int)(total % 256);
}
