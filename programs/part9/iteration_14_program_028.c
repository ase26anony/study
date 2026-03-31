/* test_hwloop_coverage.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates specific loop structures to exercise bitmap
 * intersection logic in GCC's hardware loop optimization pass.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop - this will be 'loop' */
    for (i = 0; i < n; ++i) {
        /* No code here to ensure other is subset */
        
        /* Inner loop - this will be 'other' */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = i * j;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse subset - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE int reverse_subset(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop - this will be 'other' */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 5; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (k = 0; k < i; ++k) {
            int a = i + k;
            int b = k * 3;
            int c = a ^ b;
            result ^= c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Overlapping but not subset loops
 * This should trigger the first continue (intersect but both have unique blocks)
 */
NOINLINE int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop - will be 'loop' */
    for (i = 0; i < n; ++i) {
        int a = i * 2;
        result += a;
        
    shared_label:
        /* Shared block - both loops intersect here */
        int b = result ^ i;
        asm volatile("" : : "r"(b));
        
        /* Only 'loop' has this code */
        if (i % 2) {
            result ^= 0x55;
        }
    }
    
    /* Second loop - will be 'other' */
    while (j < n) {
        int c = j * 3;
        result -= c;
        
        /* Jump into first loop's body */
        if (j == n/2) {
            goto shared_label;
        }
        
        /* Only 'other' has this code */
        if (j % 3) {
            result |= 0xAA;
        }
        
        j++;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int mixed_loops(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; i++) {
        int j = 0;
        do {
            int a = i * j;
            int b = a << 2;
            int c = b ^ j;
            result += c;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = i;
        while (k > 0) {
            result ^= k;
            asm volatile("" : : "r"(k));
            k--;
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with partial overlap via break */
NOINLINE int sibling_loops(int n) {
    int result = 0;
    int i, j;
    
    /* First loop */
    for (i = 0; i < n; i++) {
        if (i == n/2) {
            /* Break to shared code */
            goto shared_code;
        }
        result += i * 2;
    }
    
    /* Second loop */
    for (j = 0; j < n; j++) {
        result -= j * 3;
        
    shared_code:
        /* Shared block */
        int temp = result ^ (i + j);
        asm volatile("" : : "r"(temp));
        
        /* Unique to second loop */
        if (j % 4 == 0) {
            result |= 0xF0;
        }
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    int N = (seed % 100) + 10;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N);
    total ^= reverse_subset(N);
    total ^= overlapping_loops(N);
    total ^= mixed_loops(N);
    total ^= sibling_loops(N);
    
    /* Generate side effect to prevent elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
