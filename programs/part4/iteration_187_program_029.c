/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent control flow and operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 0xdeadbeef;
    
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
            c = c ^ (a * b);
            d = d & (c + 1);
        }
        
        /* Mixed operations with different data types */
        int temp = (int)(a % 256) - (int)(b % 256);
        if (temp < 0) {
            temp = -temp;
            d = d + (unsigned int)temp;
        } else {
            d = d - (unsigned int)temp;
        }
        
        /* Bitwise operations creating complex dependencies */
        unsigned int mask = 0x55555555;
        a = (a & mask) | (b & ~mask);
        b = (b & mask) | (c & ~mask);
        c = (c & mask) | (d & ~mask);
        
        /* Memory barrier via inline asm to prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Early exit condition based on computation */
        if (d > 0x80000000 && i > iterations / 2) {
            d = d % 1000;
            break;
        }
    }
    
    /* Final mixing */
    a = a ^ b ^ c ^ d;
    a = a * 0xcc9e2d51;
    a = (a << 15) | (a >> 17);
    a = a * 0x1b873593;
    
    sink = a; /* Volatile write to prevent elimination */
    return a;
}

/* Another function with nested loops */
unsigned int nested_operations(unsigned int base) {
    unsigned int result = base;
    
    for (int i = 0; i < 100; i++) {
        int inner_sum = 0;
        for (int j = 0; j < 20; j++) {
            /* Complex expression with division/modulo (expensive ops) */
            inner_sum += (result % (j + 2)) * (i + 1);
            
            /* Switch statement for varied control flow */
            switch ((i + j) % 4) {
                case 0:
                    result = result + (inner_sum << 1);
                    break;
                case 1:
                    result = result - (inner_sum >> 1);
                    break;
                case 2:
                    result = result ^ inner_sum;
                    break;
                case 3:
                    result = result | (inner_sum & 0xFFFF);
                    break;
            }
        }
        
        /* Conditional with floating point to inhibit optimizations */
        if (result % 7 == 0) {
            float f = (float)result / 7.0f;
            result = (unsigned int)(f * 100.0f);
        }
    }
    
    return result;
}

/* Main driver that uses external input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    
    /* Use command line argument if provided */
    if (argc > 1) {
        seed = (unsigned int)atoi(argv[1]);
    } else {
        /* Or read from stdin to ensure runtime execution */
        printf("Enter seed value: ");
        scanf("%u", &seed);
    }
    
    /* Call complex functions multiple times with different parameters */
    unsigned int r1 = complex_loop(seed, 1000);
    unsigned int r2 = nested_operations(seed ^ 0x12345678);
    unsigned int r3 = complex_loop(r1, 500);
    unsigned int r4 = nested_operations(r2 + r3);
    
    /* Final result depends on all computations */
    unsigned int final_result = r1 ^ r2 ^ r3 ^ r4;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u (0x%08x)\n", final_result, final_result);
    
    return (final_result % 256);
}
