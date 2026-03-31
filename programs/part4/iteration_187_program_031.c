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
    unsigned int d = 0xdeadbeef;
    
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a ^ (a >> 13)) * 0x85ebca6b;
        b = (b + (b << 7)) ^ 0x165667b1;
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            c = (c * 0xcc9e2d51) + a;
            d = d ^ (d << 15);
        } else if (i % 3 == 1) {
            c = (c ^ b) * 0x1b873593;
            d = (d + a) | 0x7fffffff;
        } else {
            c = (c + d) % 0x10000000;
            d = (d * 0xc2b2ae35) >> 3;
        }
        
        /* Bitwise operations with shifting */
        a = (a & 0x55555555) | (b & 0xaaaaaaaa);
        b = (b << 1) | (b >> 31); /* Rotate right */
        
        /* Mixed-width operations to inhibit optimization */
        long long temp = (long long)a * (long long)b;
        c = c ^ (unsigned int)(temp & 0xffffffff);
        d = d ^ (unsigned int)(temp >> 32);
        
        /* Inline asm to create memory barrier and prevent reordering */
        asm volatile("" ::: "memory");
        
        /* Conditional break based on computed value */
        if ((a & 0xfff) == 0xabc) {
            break;
        }
    }
    
    /* Final mixing */
    a = a ^ b ^ c ^ d;
    a = a * 0x85ebca6b;
    a = a ^ (a >> 16);
    
    return a;
}

/* Another function with nested loops and switch statement */
int nested_operations(int base, int count) {
    int result = base;
    volatile int local_sink = 0;
    
    for (int i = 0; i < count; i++) {
        int inner = result;
        
        for (int j = 0; j < 5; j++) {
            /* Switch with multiple cases */
            switch ((i + j) % 4) {
                case 0:
                    inner = (inner * 3) / 2;
                    break;
                case 1:
                    inner = (inner << 2) | 0x0f;
                    break;
                case 2:
                    inner = inner ^ 0x12345678;
                    inner = inner - 17;
                    break;
                case 3:
                    inner = (inner % 1000) + 1;
                    break;
            }
            
            /* More arithmetic with division (expensive operation) */
            if (inner != 0) {
                inner = 1000000 / (inner + 1);
            }
        }
        
        /* Modulo operation creates dependency chain */
        result = (result + inner) % 0x7fffffff;
        
        /* Store to volatile to prevent elimination */
        local_sink = result;
    }
    
    global_sink = local_sink;
    return result;
}

/* Function with pointer arithmetic and memory access */
int process_array(int *arr, int size) {
    int sum = 0;
    int *ptr = arr;
    int *end = arr + size;
    
    while (ptr < end) {
        /* Loop-carried dependency */
        sum = sum + *ptr;
        
        /* Complex address calculation */
        int offset = (sum & 0x3f);
        if (offset < size) {
            sum = sum ^ arr[offset];
        }
        
        /* Pointer increment with scaling */
        ptr = ptr + 1 + ((sum & 1) ? 0 : 1);
        
        /* Conditional continue */
        if ((*ptr & 0x80) && ptr < end - 1) {
            continue;
        }
        
        /* Additional computation */
        sum = (sum << 1) | (sum >> 31);
    }
    
    return sum;
}

int main(int argc, char **argv) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Initialize array with pseudo-random values */
    int array[64];
    for (int i = 0; i < 64; i++) {
        array[i] = (i * 1103515245 + 12345) & 0x7fffffff;
    }
    
    /* Call all complex functions to create scheduling opportunities */
    unsigned int hash = complex_loop(0x12345678, iterations);
    int nested_result = nested_operations(hash & 0xffff, iterations / 2);
    int array_result = process_array(array, 64);
    
    /* Combine results in a non-trivial way */
    int final_result = (hash % 1000) + nested_result - array_result;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d (hash: %u, nested: %d, array: %d)\n", 
           final_result, hash, nested_result, array_result);
    
    return final_result & 0xff;
}
