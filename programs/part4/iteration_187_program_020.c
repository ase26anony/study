/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and operations */
unsigned int compute_hash(unsigned int seed, int iterations) {
    volatile unsigned int sink; /* Prevent dead code elimination */
    unsigned int hash = seed;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        hash = (hash * 1103515245U + 12345U) & 0x7fffffffU;
        
        /* Data-dependent branching */
        if (hash % 3 == 0) {
            /* Bitwise operations */
            hash ^= (hash << 13);
            hash ^= (hash >> 17);
            hash ^= (hash << 5);
        } else if (hash % 5 == 0) {
            /* Different arithmetic mix */
            hash = (hash * 1664525U + 1013904223U);
            hash = (hash % 999983) | (hash & 0xffff);
        } else {
            /* Mixed operations */
            hash = ((hash + 1) * 3) / 2;
            hash = hash ^ 0xdeadbeef;
        }
        
        /* Nested loop with break condition */
        for (j = 0; j < 3; j++) {
            if ((hash >> j) & 1) {
                hash += j * 7919; /* Prime number operation */
                if (hash > 0xfffffff) break;
            }
        }
        
        /* Memory barrier simulation */
        asm volatile("" ::: "memory");
        
        /* More operations with different data types */
        unsigned long temp = (unsigned long)hash * 29943829UL;
        hash = (unsigned int)((temp >> 16) ^ (temp & 0xffff));
        
        /* Switch statement for control flow complexity */
        switch (hash % 7) {
            case 0: hash = hash << 1; break;
            case 1: hash = hash >> 1; break;
            case 2: hash = hash + 0x12345678; break;
            case 3: hash = hash - 0x87654321; break;
            case 4: hash = hash ^ 0x55555555; break;
            case 5: hash = hash | 0xaaaaaaaa; break;
            default: hash = hash & 0x33333333; break;
        }
    }
    
    sink = hash; /* Volatile write to prevent elimination */
    return hash;
}

/* Another function with different pattern */
int process_array(int *data, int size) {
    volatile int vsink;
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex conditional operations */
        if (val > 0) {
            sum += val * 2;
            if (sum > 1000000) {
                sum %= 1000000;
                asm volatile("" ::: "memory");
            }
        } else if (val < 0) {
            sum -= (-val) / 3;
            sum = sum ^ (i * 9973);
        } else {
            sum = (sum << 4) | (sum >> 28);
        }
        
        /* Modulo operation creates dependency */
        if (i % 4 == 0) {
            sum = (sum * 3 + 1);
        } else if (i % 4 == 1) {
            sum = sum + (val % 257);
        } else if (i % 4 == 2) {
            sum = sum | 0xff00ff;
        } else {
            sum = sum & 0x00ff00ff;
        }
        
        /* Prevent loop unrolling optimization */
        if (i == size - 1) {
            asm volatile("" ::: "memory");
        }
    }
    
    vsink = sum;
    return sum;
}

/* Main driver with external input */
int main(int argc, char **argv) {
    int i;
    unsigned int final_hash = 0x12345678;
    int data[100];
    
    /* Initialize with non-constant data */
    for (i = 0; i < 100; i++) {
        data[i] = (i * 1973) % 991;
        if (i % 2 == 0) data[i] = -data[i];
    }
    
    /* Get iterations from argument or stdin to prevent compile-time computation */
    int iterations = 1000;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    } else {
        /* scanf prevents constant propagation */
        printf("Enter number of iterations (default 1000): ");
        if (scanf("%d", &iterations) != 1) iterations = 1000;
    }
    
    /* Call complex functions multiple times */
    for (i = 0; i < 3; i++) {
        final_hash ^= compute_hash(final_hash + i, iterations / 10);
        int sum = process_array(data, 100);
        final_hash += (unsigned int)sum;
        
        /* Modify data to change pattern */
        data[i % 100] = final_hash % 1000;
    }
    
    /* Use result to prevent elimination */
    printf("Result: 0x%08x\n", final_hash);
    return (int)(final_hash & 0xff);
}
