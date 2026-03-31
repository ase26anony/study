/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate compilation units */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (loop) */
    for (i = 0; i < n; ++i) {
        /* No code here - ensures other is subset */
        
        /* Inner loop (other) - perfectly nested */
        for (j = 0; j < i + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * a;
            int e = d >> 2;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No code here either - ensures pure subset relationship */
    }
    
    return result;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (other) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in other not in loop */
        for (j = 0; j < 3; ++j) {
            int temp = i * j;
            asm volatile("" : : "r"(temp));
            result += temp;
        }
        
        /* Second inner loop (loop) - subset of other */
        for (k = 0; k < i; ++k) {
            /* Complex body for register pressure */
            int a = i * k;
            int b = a ^ k;
            int c = b << 1;
            int d = c - i;
            int e = d * a;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            result ^= (a + b + c + d + e) & 0xFF;
        }
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto (Condition 1) */
NOINLINE int overlapping_with_goto(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A (loop) */
    for (i = 0; i < n; ++i) {
        int a = i * 3;
        asm volatile("" : : "r"(a));
        
    shared_block:
        result += a & 0xF;
        
        /* Loop B (other) - shares block via goto */
        while (j < n) {
            int b = j * 5;
            asm volatile("" : : "r"(b));
            
            if (b > a * 2) {
                /* Jump into loop A's body */
                goto shared_block;
            }
            
            result ^= b;
            j++;
        }
    }
    
    return result;
}

/* Function 4: Mixed loop types for varied CFG */
NOINLINE int mixed_loop_types(int n) {
    int result = 0;
    int i = 0;
    
    /* do-while inside for */
    for (i = 0; i < n; ++i) {
        int j = 0;
        
        /* do-while loop (other) */
        do {
            int a = i + j;
            int b = a * j;
            int c = b ^ i;
            
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result += c & 0x7F;
            j++;
        } while (j < 5);
        
        /* while loop after do-while */
        int k = 0;
        while (k < i) {
            int d = k * i;
            asm volatile("" : : "r"(d));
            result ^= d;
            k++;
        }
    }
    
    return result;
}

/* Function 5: Complex nesting with break to shared label */
NOINLINE int complex_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (loop) */
    for (i = 0; i < n; ++i) {
        /* Label inside loop for goto target */
    inside_loop:
        result += i * 2;
        
        /* Inner loop (other) with break to shared location */
        for (j = 0; j < n; ++j) {
            int a = i * j;
            int b = a ^ 0x55;
            
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
            
            if (a > n * 2) {
                /* Break to a location inside outer loop */
                goto inside_loop;
            }
        }
        
        /* More code in outer loop after inner loop */
        int c = result * 3;
        asm volatile("" : : "r"(c));
        result = c & 0xFFF;
    }
    
    return result;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Use volatile and argc to create variable loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_with_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= complex_nesting(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    return total & 1;
}
