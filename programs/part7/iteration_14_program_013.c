/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Target: ARMv8-A with hardware loop support
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile assembly to prevent optimization */
#define KEEP(var) asm volatile("" : : "r"(var))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 50) + 10;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < limit; ++i) {
        /* No code here - ensures other's blocks are subset of loop's blocks */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (int j = 0; j < i + 5; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a ^ j;
            int c = b - i;
            int d = c * 3;
            int e = d >> 2;
            result ^= (a + b - c) * e;
            KEEP(result);
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    /* Add some code after to create blocks in loop but not in other */
    result += limit * 2;
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < limit; ++i) {
        /* First inner loop - creates blocks in other but not in loop */
        for (int j = 0; j < 3; ++j) {
            int a = i * j;
            int b = a ^ 0x55;
            result += b;
            KEEP(result);
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (int k = 0; k < i + 2; ++k) {
            /* This loop's blocks are subset of other's blocks */
            int x = k * 7;
            int y = x ^ k;
            int z = y - i;
            result ^= z;
            KEEP(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A (will be 'loop' in hierarchy) */
    for (int i = 0; i < limit; ++i) {
        int a = i * 3;
        
    loop_body:
        result += a;
        KEEP(result);
        
        /* Loop B (will be 'other' in hierarchy) */
        for (int j = 0; j < 5; ++j) {
            int b = j * 2;
            result ^= b;
            
            /* Jump into loop A's body, creating intersection */
            if (j == 3 && i % 2 == 0) {
                a = b;
                goto loop_body;
            }
            
            KEEP(result);
        }
        
        result -= i;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 10;
    
    /* for loop */
    for (int i = 0; i < limit; ++i) {
        int a = i;
        
        /* do-while loop inside */
        int counter = 0;
        do {
            int b = a * counter;
            int c = b ^ 0xAA;
            result += c;
            counter++;
            KEEP(result);
        } while (counter < 3);
        
        /* while loop after */
        int w = 0;
        while (w < 2) {
            result ^= (a << w);
            w++;
            KEEP(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE int complex_sibling_loops(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 5;
    
    /* Outer loop */
    for (int outer = 0; outer < limit; ++outer) {
        /* First sibling inner loop */
        for (int inner1 = 0; inner1 < outer + 1; ++inner1) {
            int x = outer * inner1;
            int y = x ^ 0xCC;
            result += y;
            KEEP(result);
        }
        
        /* Some intermediate code */
        result ^= outer;
        
        /* Second sibling inner loop */
        for (int inner2 = 0; inner2 < 3; ++inner2) {
            int z = outer + inner2;
            result -= z * 2;
            KEEP(result);
        }
        
        /* Third loop with break to label */
        for (int k = 0; k < 4; ++k) {
            if (k == 2 && outer % 3 == 0) {
                break;
            }
            result |= k;
            KEEP(result);
        }
    }
    
    return result & 0xFF;
}

/* Main function to drive execution */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int base = argc;
    
    /* Call each function with different arguments */
    total ^= perfect_nesting(base + 1);
    total ^= loop_subset_of_other(base + 2);
    total ^= partial_overlap_goto(base + 3);
    total ^= mixed_loop_types(base + 4);
    total ^= complex_sibling_loops(base + 5);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
