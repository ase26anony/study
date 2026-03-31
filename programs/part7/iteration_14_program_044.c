/* 
 * Test program for hardware loop optimization coverage in GCC.
 * Specifically targets bitmap intersection logic in hw-doloop.cc lines 429-436.
 * 
 * Compilation for hardware loop targets:
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c hwloop_test.c -o hwloop_test.o
 *   gcc -fprofile-arcs hwloop_test.o -o hwloop_test
 *   ./hwloop_test
 *   gcov -b hwloop_test.c
 *
 * For generic targets with loop optimization:
 *   gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage hwloop_test.c -o hwloop_test
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Create register pressure with multiple variables */
#define CREATE_REG_PRESSURE(i) \
    int a = (i) ^ 0x55AA55AA; \
    int b = a * 3 + 7; \
    int c = b >> 4; \
    int d = c - a; \
    int e = d * b; \
    int f = e ^ c; \
    result = (result * 31 + f) & 0xFFFF;

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* No code here ensures loop has no blocks outside other */
        
        /* Inner loop - this will be 'other' in the analysis */
        for (int j = 0; j < (N - i); ++j) {
            CREATE_REG_PRESSURE(j);
            /* Prevent optimization */
            asm volatile("" : : "r"(result));
        }
        
        /* No code here either - ensures perfect nesting */
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other (Condition 3) */
NOINLINE
int reverse_nesting(int N) {
    int result = 0;
    
    /* Outer loop - this will be 'other' in the analysis */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 3; ++j) {
            int temp = i * j;
            result ^= temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Second inner loop - this will be 'loop' in the analysis */
        for (int k = 0; k < (N / 2); ++k) {
            CREATE_REG_PRESSURE(k);
            asm volatile("" : : "r"(result));
        }
    }
    
    return result;
}

/* Function 3: Partial overlap with goto - triggers first condition */
NOINLINE
int partial_overlap_goto(int N) {
    int result = 0;
    int i = 0, j = 0;
    
    /* Loop A - will be 'loop' in analysis */
loop_a:
    for (i = 0; i < N; ++i) {
        CREATE_REG_PRESSURE(i);
        
        /* Loop B - will be 'other' in analysis */
        for (j = 0; j < 5; ++j) {
            CREATE_REG_PRESSURE(j);
            
            /* Conditional goto into loop A's body */
            if ((i + j) % 7 == 0) {
                /* This creates shared basic blocks */
                goto shared_block;
            }
        }
        
        continue;
        
shared_block:
        /* Shared block between both loops */
        result ^= (i << 16) | j;
        asm volatile("" : : "r"(result));
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex CFG */
NOINLINE
int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        int j = 0;
        
        /* do-while loop inside while */
        do {
            CREATE_REG_PRESSURE(j);
            j++;
        } while (j < 5);
        
        /* for loop after do-while */
        for (int k = 0; k < 3; ++k) {
            result = (result + i * k) & 0xFF;
            asm volatile("" : : "r"(result));
        }
        
        i++;
    }
    
    /* Another for loop that shares some blocks via break */
    for (int x = 0; x < N; ++x) {
        if (x % 3 == 0) {
            /* This creates partial overlap */
            result ^= x;
            break;
        }
        CREATE_REG_PRESSURE(x);
    }
    
    return result;
}

/* Function 5: Sibling loops with shared exit block */
NOINLINE
int sibling_loops(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        if (i % 2 == 0) {
            CREATE_REG_PRESSURE(i);
        }
    }
    
    /* Shared variable used by both loops */
    int shared = result;
    
    /* Second loop that shares exit block with first */
    for (int j = 0; j < N * 2; ++j) {
        shared += j;
        CREATE_REG_PRESSURE(j);
    }
    
    /* Common exit code block */
    result = shared & 0xFF;
    asm volatile("" : : "r"(result));
    
    return result;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create variable loop bounds */
    volatile int seed = argc + global_seed;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 30) + 5;
    int N3 = (seed % 40) + 8;
    int N4 = (seed % 20) + 3;
    int N5 = (seed % 60) + 15;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= partial_overlap_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops(N5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 255);
    
    /* Additional calls with different parameters */
    if (argc > 1) {
        total ^= perfect_nesting(atoi(argv[1]) % 100);
        total ^= reverse_nesting(atoi(argv[1]) % 80);
        printf("Additional result: %d\n", total & 255);
    }
    
    return total & 1;
}
