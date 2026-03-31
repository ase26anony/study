/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 1;
    
    /* Loop with multiple arithmetic and bitwise operations */
    for (int i = 0; i < iterations; i++) {
        /* Data-dependent branching */
        if (i % 3 == 0) {
            a = (a ^ (a >> 7)) * 0x45d9f3b;
            b = (b + 0x9e3779b9) ^ a;
        } else if (i % 3 == 1) {
            a = (a * 1664525) + 1013904223;
            b = b ^ (b << 13);
        } else {
            a = a + (a << 16);
            b = (b * 1103515245) + 12345;
        }
        
        /* More operations with different data types */
        c = c + (a & 0xFF);
        d = d ^ (b >> 8);
        
        /* Nested conditional with bitwise operations */
        if ((c & 0xF) == 0) {
            d = d * 3;
        } else if ((c & 0xF) < 8) {
            d = d + (d << 2);
        } else {
            d = d | 0x55555555;
        }
        
        /* Mixed arithmetic operations */
        int temp = (int)(c % 100) - 50;
        if (temp > 0) {
            d = d + (unsigned int)temp;
        } else {
            d = d - (unsigned int)(-temp);
        }
        
        /* Artificial memory barrier */
        asm volatile("" ::: "memory");
    }
    
    /* Combine results with complex expression */
    unsigned int result = (a ^ b) + (c * d);
    
    /* Prevent optimization */
    sink = result;
    return result;
}

/* Another function with different pattern */
unsigned int nested_loop_computation(unsigned int base) {
    unsigned int sum = 0;
    
    /* Outer loop */
    for (int i = 0; i < 50; i++) {
        unsigned int val = base + i;
        
        /* Inner loop with break condition */
        for (int j = 0; j < 20; j++) {
            if (val > 1000000) break;
            
            /* Multiple operations in inner loop */
            val = (val * 6364136223846793005ULL) + 1442695040888963407ULL;
            val = val ^ (val >> 21);
            val = val ^ (val << 35);
            val = val ^ (val >> 4);
            
            /* Conditional operation */
            if (j % 4 == 0) {
                sum += val & 0xFF;
            } else if (j % 4 == 1) {
                sum += (val >> 8) & 0xFF;
            } else if (j % 4 == 2) {
                sum += (val >> 16) & 0xFF;
            } else {
                sum += (val >> 24) & 0xFF;
            }
            
            /* Complex expression with division (expensive) */
            if (val % 7 == 0) {
                sum = sum / 2;
            }
        }
        
        /* Loop-carried dependency */
        base = base ^ sum;
    }
    
    return sum;
}

/* Function with switch statement */
int switch_based_computation(int x) {
    int result = 0;
    
    for (int i = 0; i < 100; i++) {
        switch (x % 5) {
            case 0:
                result += i * 3;
                x = x ^ (i << 3);
                break;
            case 1:
                result -= i * 5;
                x = x + (i * 7);
                break;
            case 2:
                result |= i;
                x = x * 11;
                break;
            case 3:
                result &= ~i;
                x = x - 19;
                break;
            case 4:
                result ^= i * 13;
                x = x / 3;
                break;
        }
        
        /* Prevent pattern recognition */
        if (result < 0) {
            result = -result;
        }
        
        /* Memory clobber */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    unsigned int seed = 12345;
    
    /* Use command line argument for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    /* Call complex functions multiple times */
    unsigned int total = 0;
    
    for (int k = 0; k < 3; k++) {
        total += complex_loop(seed + k * 100, 100);
        total += nested_loop_computation(seed + k * 200);
        total += switch_based_computation(seed + k * 300);
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245) + 12345;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", total);
    
    return total % 256;
}
