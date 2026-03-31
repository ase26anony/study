/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
static unsigned int complex_loop(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed;
    unsigned int b = 0x9e3779b9; /* Golden ratio */
    unsigned int c = 0;
    unsigned int d = 1;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x85ebca6b;
        b = (b ^ (b << 17)) ^ 0xc2b2ae35;
        c = c + (a * b) / (i + 1);
        d = d | (c << (i % 16));
        
        /* Data-dependent branching */
        if (a % 3 == 0) {
            c = c ^ (d >> 4);
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        } else if (a % 7 == 0) {
            d = d * 0x5bd1e995;
            b = b ^ d;
        } else {
            a = a + (c % 256);
            /* Another memory barrier */
            asm volatile("" ::: "memory");
        }
        
        /* More operations with different data types */
        int temp = (int)(a & 0xFF);
        if (temp < 128) {
            d = d << 1;
        } else {
            d = d >> 1;
        }
        
        /* Loop-carried dependency */
        b = b + c;
        
        /* Switch statement for complex control flow */
        switch (i % 5) {
            case 0: a = a * 3; break;
            case 1: b = b / 2; break;
            case 2: c = c | a; break;
            case 3: d = d ^ b; break;
            case 4: a = a + d; break;
        }
    }
    
    /* Mix results to prevent optimization */
    unsigned int result = (a ^ b) + (c * d);
    sink = result; /* Volatile write */
    
    return result;
}

/* Another function with nested loops */
static unsigned int nested_loops(unsigned int init) {
    unsigned int sum = init;
    
    for (int i = 0; i < 100; i++) {
        int limit = (i % 10) + 5;
        for (int j = 0; j < limit; j++) {
            /* Various operations */
            sum = sum + (i * j);
            sum = sum ^ (sum << 3);
            sum = sum * 0xcc9e2d51;
            
            /* Conditional break */
            if (sum > 0x80000000) {
                sum = sum >> 1;
                if (j > limit/2) break;
            }
        }
        
        /* More operations between loops */
        sum = sum - (i * 0x123456);
        asm volatile("" ::: "memory"); /* Memory barrier */
    }
    
    return sum;
}

/* Function with pointer arithmetic and memory access */
static unsigned int pointer_ops(unsigned int *arr, int size) {
    unsigned int hash = 0x811c9dc5;
    
    for (int i = 0; i < size; i++) {
        hash = hash ^ arr[i];
        hash = hash * 0x01000193;
        
        /* Complex condition */
        if ((hash & 0xF) == 0) {
            hash = hash + (i << 8);
        } else if ((hash & 0xF0) == 0) {
            hash = hash - (arr[i] % 256);
        }
        
        /* Division/modulo operations (expensive) */
        if (i % 3 == 0) {
            hash = hash / (hash % 256 + 1);
        }
    }
    
    return hash;
}

int main(int argc, char **argv) {
    /* Use command line argument or default */
    int iterations = (argc > 1) ? atoi(argv[1]) : 1000;
    
    /* Initialize array for pointer operations */
    unsigned int array[256];
    for (int i = 0; i < 256; i++) {
        array[i] = i * 0x1234567;
    }
    
    /* Call complex functions multiple times */
    unsigned int result1 = complex_loop(0xdeadbeef, iterations);
    unsigned int result2 = nested_loops(result1);
    unsigned int result3 = pointer_ops(array, 256);
    
    /* Mix all results */
    unsigned int final_result = result1 ^ result2 ^ result3;
    
    /* Print to prevent optimization */
    printf("Result: 0x%08x\n", final_result);
    
    return (final_result == 0) ? 0 : 1;
}
