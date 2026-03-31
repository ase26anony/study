/* test_sel_sched.c - Test case for GCC selective scheduling debug coverage */
#include <stdio.h>
#include <stdlib.h>

/* Complex function with data-dependent branching and mixed operations */
unsigned long complex_loop(unsigned seed, int iterations) {
    volatile unsigned sink; /* Prevent dead code elimination */
    unsigned long a = seed;
    unsigned long b = 0x9e3779b97f4a7c15ULL;
    unsigned long c = 0xdeadbeefcafebabeULL;
    unsigned long result = 0;
    
    /* Loop with data-dependent control flow and mixed operations */
    for (int i = 0; i < iterations; i++) {
        /* Multiple arithmetic operations creating dependencies */
        a = (a * 1103515245 + 12345) & 0x7fffffff;
        b = b ^ (b >> 27);
        b = (b * 0x5bd1e995) + 0x5bd1e995;
        c = c + (c << 13);
        c = c ^ (c >> 17);
        
        /* Control flow based on computed values */
        if ((a % 7) == 0) {
            result += (a & 0xff) * (b & 0xff);
            /* Inline asm to create memory barrier */
            asm volatile("" ::: "memory");
        } else if ((a % 13) == 0) {
            result += (a >> 8) | (b << 8);
            /* Bitwise operations */
            c = (c & 0x55555555) + ((c >> 1) & 0x55555555);
        } else {
            result += (a % 19) * (c % 23);
            /* Mixed integer operations */
            b = (b / 3) * 2 + (b % 3);
        }
        
        /* Nested conditional with different operation types */
        switch (i % 5) {
            case 0: result ^= a; break;
            case 1: result += b * 3; break;
            case 2: result -= c / 2; break;
            case 3: result |= (a ^ b ^ c); break;
            case 4: result &= ~(a | b | c); break;
        }
        
        /* Loop-carried dependency */
        result = result + i;
        
        /* Volatile write to prevent optimization */
        sink = result & 1;
    }
    
    /* Additional control flow outside loop */
    if (result > 1000000) {
        result = result % 1000000;
    } else {
        result = result * 3 + 1;
    }
    
    return result;
}

/* Another function with different pattern */
int nested_loops(int start, int end) {
    int sum = 0;
    volatile int vsink;
    
    for (int i = start; i < end; i++) {
        int inner = i * 2;
        for (int j = 0; j < 10; j++) {
            /* Complex expression with division (expensive) */
            inner = (inner * 3 + j) / 2;
            
            /* Conditional break */
            if (inner > 1000) {
                inner = inner % 1000;
                if (j > 5) break;
            }
            
            /* Bit manipulation */
            inner = inner ^ (inner << 3);
            inner = inner & 0x3ff;
        }
        sum += inner;
        
        /* Memory clobber to force dependencies */
        asm volatile("" : "+r"(inner) :: "memory");
    }
    
    vsink = sum;
    return sum;
}

/* Function with pointer arithmetic and memory access */
void process_array(int *arr, int size) {
    volatile int vtmp;
    int *ptr = arr;
    
    for (int i = 0; i < size; i++) {
        /* Pointer arithmetic with offset */
        int val = *(ptr + i);
        
        /* Data-dependent computation */
        if (val > 0) {
            val = (val * 2) - (val / 3);
        } else {
            val = (val << 2) | 0x1;
        }
        
        /* Store back with barrier */
        asm volatile("" ::: "memory");
        arr[i] = val;
        
        /* Volatile access */
        vtmp = val;
    }
}

int main(int argc, char **argv) {
    unsigned long total = 0;
    int iterations = 100;
    
    /* Use argument to prevent compile-time computation */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 10) iterations = 10;
        if (iterations > 1000) iterations = 1000;
    }
    
    /* Call complex functions multiple times */
    for (int i = 0; i < 3; i++) {
        total += complex_loop(i * 12345 + 6789, iterations);
        total += nested_loops(i * 10, i * 10 + 20);
    }
    
    /* Array processing */
    int array[50];
    for (int i = 0; i < 50; i++) {
        array[i] = (i * 3) % 47;
    }
    process_array(array, 50);
    
    /* Use array result */
    for (int i = 0; i < 50; i++) {
        total += array[i];
    }
    
    /* Final result depends on all computations */
    printf("Result: %lu\n", total);
    
    return (int)(total % 256);
}
