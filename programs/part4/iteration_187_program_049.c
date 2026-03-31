/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned int seed, int iterations) {
    volatile unsigned long state = seed; /* Prevent optimization */
    unsigned long result = 0;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        unsigned long temp = state;
        
        /* Inner loop with multiple operations */
        for (j = 0; j < 8; j++) {
            /* Data-dependent branching */
            if (temp & 1) {
                /* Mix of arithmetic operations */
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                temp = (temp >> 16) | (temp << 16);
                temp ^= 0xDEADBEEF;
            } else {
                /* Different operations for even values */
                temp = (temp * 1664525 + 1013904223) % 0x7fffffff;
                temp = (temp << 3) | (temp >> 29);
                temp &= 0xFFFFFFFF;
            }
            
            /* More operations to increase instruction count */
            temp = (temp + i) * (j + 1);
            temp = temp - (temp / 3);
            temp = temp | (temp << 8);
            temp = temp ^ (temp >> 4);
            
            /* Conditional break based on computed value */
            if ((temp & 0xFF) == 0 && j > 2)
                break;
        }
        
        /* Create dependencies between iterations */
        state = temp ^ (state << 1);
        result += state;
        
        /* More control flow */
        switch (i % 4) {
            case 0:
                result = result * 3;
                break;
            case 1:
                result = result + (result >> 2);
                break;
            case 2:
                result = result ^ 0x12345678;
                break;
            case 3:
                result = result - (result / 5);
                break;
        }
        
        /* Memory barrier simulation */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Another function with different pattern to increase scheduling complexity */
int nested_branching(int x, int y) {
    int z = 0;
    volatile int counter = 0;
    
    while (x > 0 && y > 0) {
        /* Complex conditional chain */
        if (x % 2 == 0) {
            if (y % 3 == 0) {
                z = (z << 1) + x;
                x = x / 2;
            } else if (y % 5 == 0) {
                z = (z >> 1) ^ y;
                y = y - 1;
            } else {
                z = z * 3 - 1;
                x = x - y;
            }
        } else {
            switch (y % 4) {
                case 0:
                    z = z + (x * y);
                    x--;
                    break;
                case 1:
                    z = z | (x & y);
                    y >>= 1;
                    break;
                case 2:
                    z = z ^ (x | y);
                    x = x ^ y;
                    break;
                case 3:
                    z = (z + x) % (y + 1);
                    y = y ^ x;
                    break;
            }
        }
        
        /* Prevent infinite loops */
        counter++;
        if (counter > 1000) break;
        
        /* More operations with different data types */
        long l = (long)z * (long)x;
        z = (int)(l % 1000000);
        
        /* Bitwise operations */
        z = (z & 0x5555) | ((z & 0xAAAA) >> 1);
    }
    
    return z;
}

/* Main function that uses both complex functions */
int main(int argc, char *argv[]) {
    unsigned int seed = 42;
    int iterations = 100;
    
    /* Use command line arguments to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    /* Call complex functions multiple times */
    unsigned long total = 0;
    for (int i = 0; i < 3; i++) {
        total += complex_loop(seed + i, iterations);
        total ^= nested_branching(seed + i * 7, iterations / 2);
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %lu\n", total);
    
    /* Additional computation with array access */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i * i - i;
    }
    
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        if (i % 3 == 0) {
            sum += array[i] * 2;
        } else if (i % 3 == 1) {
            sum += array[i] / 2;
        } else {
            sum += array[i] ^ total;
        }
    }
    
    printf("Array sum: %d\n", sum);
    
    return 0;
}
