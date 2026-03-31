/* test_sel_sched.c
 * Compile with: gcc -O3 -fselective-scheduling2 -fsel-sched-dump -c test_sel_sched.c -o test.o
 * Additional flags for more debug output: -fdump-rtl-sched1 -fdump-rtl-sched2
 */

#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent control flow and mixed operations */
static unsigned long compute_hash(unsigned int seed, int iterations) {
    volatile unsigned long hash = seed; /* volatile to prevent optimization */
    unsigned long temp;
    int i, j;
    
    /* Outer loop with loop-carried dependency */
    for (i = 0; i < iterations; i++) {
        /* Inner loop with complex operations */
        for (j = 0; j < 32; j++) {
            /* Data-dependent branching */
            if ((i + j) % 3 == 0) {
                hash = (hash * 1103515245 + 12345) & 0x7fffffff;
                hash ^= (hash << 13) | (hash >> 19);
            } else if ((i + j) % 5 == 0) {
                hash = (hash * 1664525 + 1013904223) & 0xffffffff;
                hash = (hash + i * j) | (hash << 7);
            } else {
                hash = (hash ^ (hash >> 15)) * 0x45d9f3b;
                hash = hash ^ (hash << 11);
            }
            
            /* Mixed arithmetic operations */
            temp = hash % 7919; /* Prime modulus */
            hash = (hash + temp * 3) / 2;
            hash = hash - (temp << 4);
            
            /* Bitwise operations */
            hash = (hash & 0xAAAAAAAA) | (hash & 0x55555555) << 1;
            hash = hash ^ (i << (j % 16));
            
            /* Memory barrier via inline asm */
            asm volatile("" ::: "memory");
            
            /* Early exit condition based on computed value */
            if (hash % 10007 == 0 && j > 16) {
                break;
            }
        }
        
        /* Additional control flow in outer loop */
        switch (i % 7) {
            case 0: hash = hash * 3; break;
            case 1: hash = hash + 0x12345678; break;
            case 2: hash = hash ^ 0x87654321; break;
            case 3: hash = hash | 0xF0F0F0F0; break;
            case 4: hash = hash & 0x0F0F0F0F; break;
            case 5: hash = hash << 3; break;
            case 6: hash = hash >> 2; break;
        }
    }
    
    return hash;
}

/* Another function with different pattern to increase scheduling complexity */
static long process_array(const int* data, int size) {
    volatile long sum = 0;
    long product = 1;
    int i;
    
    for (i = 0; i < size; i++) {
        int val = data[i];
        
        /* Complex dependency chain */
        if (val > 0) {
            sum += val * 2;
            product *= (val % 17) + 1;
            
            /* Floating point to integer conversion adds complexity */
            double dval = val * 0.314159;
            sum += (long)dval;
        } else if (val < 0) {
            sum -= (-val) << 1;
            product /= (-val % 13) + 1;
            
            /* More arithmetic */
            sum = sum ^ (sum >> 3);
            product = product & 0x00FFFFFF;
        } else {
            /* Zero case - different operations */
            sum = sum | 0xFF;
            product = product ^ product;
        }
        
        /* Nested conditional */
        if (i % 11 == 0) {
            for (int k = 0; k < 3; k++) {
                sum += k * product;
                asm volatile("" ::: "memory"); /* Another memory barrier */
            }
        }
    }
    
    /* Final mixing */
    return sum ^ product;
}

/* Main driver that uses both functions */
int main(int argc, char** argv) {
    int iterations = 100;
    int array_size = 50;
    unsigned int seed = 42;
    int* data;
    int i;
    
    /* Use command line argument to prevent compile-time computation */
    if (argc > 1) {
        seed = atoi(argv[1]);
        if (seed == 0) seed = 42;
    }
    
    /* Dynamic allocation prevents optimization */
    data = (int*)malloc(array_size * sizeof(int));
    if (!data) return 1;
    
    /* Initialize with pattern */
    for (i = 0; i < array_size; i++) {
        data[i] = (i * 37 + seed) % 100 - 50; /* Range -50 to 49 */
    }
    
    /* Call both complex functions multiple times */
    unsigned long hash1 = compute_hash(seed, iterations);
    unsigned long hash2 = compute_hash(hash1 & 0xFFFF, iterations / 2);
    
    long array_result = process_array(data, array_size);
    long array_result2 = process_array(data, array_size / 2);
    
    /* Mix results to produce final output */
    unsigned long final_hash = hash1 ^ hash2 ^ (unsigned long)array_result;
    final_hash = final_hash ^ (final_hash >> 16);
    
    /* Use result to prevent dead code elimination */
    printf("Result: 0x%08lx (array: %ld)\n", final_hash & 0xFFFFFFFF, 
           array_result + array_result2);
    
    free(data);
    return 0;
}
