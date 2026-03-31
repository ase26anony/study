/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and multiple operations */
unsigned int compute_hash(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int a = seed * 1103515245 + 12345;
    unsigned int b = seed ^ 0xDEADBEEF;
    unsigned int c = 1;
    unsigned int result = 0;
    
    /* Complex loop with data-dependent control flow */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1664525 + 1013904223) % 0xFFFFFFFF;
        b = (b ^ (b >> 17)) * 0x9DDFEA08;
        c = c + (a & 0xFF) - (b & 0xFF);
        
        /* Data-dependent branching */
        if (i % 3 == 0) {
            result += (a >> 16) & 0xFFFF;
            c = c ^ (result << 3);
        } else if (i % 5 == 0) {
            result += (b >> 8) & 0xFF;
            c = (c * 1103515245) ^ result;
        } else {
            result += a + b + c;
            /* Bitwise operations */
            result = (result << 1) | (result >> 31); /* Rotate right */
        }
        
        /* More operations with different data types */
        long temp = (long)a * (long)b;
        result ^= (unsigned int)(temp & 0xFFFFFFFF);
        
        /* Artificial memory barrier */
        asm volatile("" ::: "memory");
        
        /* Nested conditional with early exit possibility */
        if (result > 0x7FFFFFFF) {
            result = result / 2;
            if (i > iterations / 2) {
                /* Early break under specific condition */
                break;
            }
        }
    }
    
    /* Final mixing */
    result = result ^ (result >> 16);
    result = result * 0x85EBCA6B;
    result = result ^ (result >> 13);
    result = result * 0xC2B2AE35;
    result = result ^ (result >> 16);
    
    sink = result; /* Volatile write to prevent elimination */
    return result;
}

/* Second function with different pattern */
unsigned int process_array(const unsigned int* data, int size) {
    unsigned int sum = 0;
    unsigned int prod = 1;
    
    for (int i = 0; i < size; i++) {
        unsigned int val = data[i];
        
        /* Switch statement for control flow complexity */
        switch (val % 7) {
            case 0:
                sum += val * 2;
                prod *= (val | 0x1);
                break;
            case 1:
                sum += val >> 1;
                prod *= (val & 0xFFFFFFFE);
                break;
            case 2:
                sum += val + prod;
                prod = (prod << 3) ^ val;
                break;
            case 3:
                sum += val - prod;
                prod = (prod >> 2) | val;
                break;
            case 4:
                sum += val ^ prod;
                prod = prod % (val | 1);
                break;
            default:
                sum += val;
                prod = prod ^ val;
                break;
        }
        
        /* Loop-carried dependency */
        if (i > 0) {
            sum += data[i-1] % 256;
        }
        
        /* More arithmetic */
        sum = (sum * 13 + 17) % 0xFFFF;
    }
    
    return sum ^ prod;
}

/* Third function with nested loops */
unsigned int matrix_ops(int n) {
    unsigned int total = 0;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int idx = i * n + j;
            unsigned int val = idx * 137;
            
            /* Complex expression with multiple operations */
            total += ((val << (i % 8)) | (val >> (8 - (i % 8)))) 
                   * ((val >> (j % 8)) | (val << (8 - (j % 8))));
            
            /* Conditional with modulo - expensive operation */
            if ((val % (i + j + 1)) == 0) {
                total -= val / ((i + j) | 1);
            }
            
            /* Prevent optimization */
            asm volatile("" : "+r" (total) : : "memory");
        }
        
        /* Outer loop control */
        if (total > 1000000) {
            total = total % 1000000;
        }
    }
    
    return total;
}

int main(int argc, char** argv) {
    unsigned int final_result = 0;
    
    /* Use command line or default iterations */
    int iterations = (argc > 1) ? atoi(argv[1]) : 100;
    if (iterations < 10) iterations = 10;
    if (iterations > 1000) iterations = 1000;
    
    /* Initialize some data */
    unsigned int data[50];
    for (int i = 0; i < 50; i++) {
        data[i] = i * 7919; /* Prime multiplier */
    }
    
    /* Call all functions to create diverse scheduling opportunities */
    final_result ^= compute_hash(0x12345678, iterations);
    final_result ^= process_array(data, 50);
    final_result ^= matrix_ops(10);
    
    /* Another call with different parameters */
    final_result += compute_hash(final_result, iterations / 2);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %u\n", final_result);
    
    return (int)(final_result % 256);
}
