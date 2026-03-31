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
        
        /* Inner loop with mixed operations */
        for (j = 0; j < 8; j++) {
            /* Data-dependent branching */
            if (temp & 1) {
                /* Multiple arithmetic operations */
                temp = (temp * 1103515245 + 12345) & 0x7fffffff;
                temp = (temp >> 16) | (temp << 16);
                temp ^= 0xDEADBEEF;
            } else {
                /* Different operations for even bits */
                temp = (temp + 0xABCDEF) * 1664525;
                temp = (temp % 9973) | (temp & 0xFFFF);
            }
            
            /* Bitwise operations creating dependencies */
            temp = (temp << 3) ^ (temp >> 5);
            temp += (temp & 0xFF) * 0x10001;
            
            /* Conditional break based on computation */
            if ((temp & 0xFFF) == 0 && j > 2)
                break;
        }
        
        /* Mix result with temp using various operations */
        result ^= temp;
        result = (result * 13 + (result >> 17)) % 1000000007;
        
        /* Update state with dependency */
        state = temp + i;
        
        /* Additional conditional operation */
        if (i % 7 == 0) {
            result = (result << 1) | (result >> 31);
            result ^= 0x12345678;
        } else if (i % 13 == 0) {
            result = (result / 3) + (result % 1024);
        }
        
        /* Memory barrier simulation */
        asm volatile("" ::: "memory");
    }
    
    return result;
}

/* Another function with switch statement for control flow complexity */
int switch_based_computation(int x, int y) {
    int result = 0;
    volatile int counter = x; /* Prevent optimization */
    
    for (int i = 0; i < y; i++) {
        switch (counter % 5) {
            case 0:
                result += (counter * 3) / 2;
                counter = (counter << 2) ^ 0x55;
                break;
            case 1:
                result -= (counter % 100) * 7;
                counter = (counter + 12345) & 0xFFF;
                break;
            case 2:
                result ^= (counter | 0xF0F0F0F0);
                counter = (counter * 1664525) % 1000000;
                break;
            case 3:
                result = (result >> 4) | (result << 28);
                counter = counter / 3 + 1;
                break;
            case 4:
                result = result * 2 - counter;
                counter = (counter ^ 0xAAAAAAAA) + i;
                break;
        }
        
        /* Additional operation to create more scheduling opportunities */
        if (result < 0) {
            result = -result;
            counter += result % 256;
        }
        
        /* Prevent loop unrolling */
        asm volatile("" : "+r"(counter) : : "memory");
    }
    
    return result;
}

/* Main driver that uses volatile input to prevent compile-time computation */
int main(int argc, char *argv[]) {
    unsigned int seed = 42;
    int iterations = 100;
    
    /* Use command line arguments to prevent constant propagation */
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
    for (int run = 0; run < 3; run++) {
        unsigned long r1 = complex_loop(seed + run, iterations);
        int r2 = switch_based_computation(seed + run * 7, iterations / 10 + 5);
        
        /* Mix results with different operations */
        total += r1;
        total ^= (unsigned long)r2 << 16;
        total = (total * 6364136223846793005ULL) + 1442695040888963407ULL;
        
        /* Print intermediate to prevent dead code elimination */
        if (run == 1) {
            printf("Intermediate: %lu\n", total & 0xFFFF);
        }
    }
    
    /* Final result depends on all computations */
    printf("Final result: %lu\n", total);
    
    return (int)(total % 256);
}
