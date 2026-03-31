/* 
 * Test program for hardware loop bitmap intersection coverage
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * This test creates specific loop structures to exercise the bitmap
 * intersection logic in hw-doloop.cc lines 429-436
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))
#define VOLATILE_DO(x) asm volatile("" : : "r"(x) : "memory")

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 50) + 10;
    
    /* Outer loop (will be 'loop' in the analysis) */
    for (int i = 0; i < limit; ++i) {
        VOLATILE_DO(i);
        
        /* Inner loop (will be 'other' in the analysis) */
        /* This creates condition: other ⊂ loop */
        for (int j = 0; j < i + 1; ++j) {
            int a = i * j;
            int b = a + j;
            int c = b - a;
            result ^= (a * b) >> (c & 3);
            VOLATILE_DO(result);
        }
        
        /* No code here ensures loop has no blocks outside other */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 40) + 15;
    
    /* Outer loop (will be 'other' in the analysis) */
    for (int i = 0; i < limit; ++i) {
        VOLATILE_DO(i);
        
        /* First inner loop - creates blocks in other not in loop */
        for (int j = 0; j < 3; ++j) {
            result += i * j;
            VOLATILE_DO(result);
        }
        
        /* Second inner loop (will be 'loop' in the analysis) */
        /* This creates condition: loop ⊂ other */
        for (int k = 0; k < i + 2; ++k) {
            int a = i + k;
            int b = a * k;
            int c = b >> 1;
            result ^= (a + b) * c;
            VOLATILE_DO(result);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
NOINLINE int partial_overlap_goto(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 30) + 20;
    
    /* Loop A */
    for (int i = 0; i < limit; ++i) {
        VOLATILE_DO(i);
        
        /* Loop B - shares block via goto */
        for (int j = 0; j < 5; ++j) {
            if (j == 3 && i > limit/2) {
                goto shared_block;
            }
            result += i * j;
            VOLATILE_DO(result);
        }
        
        continue;
        
    shared_block:
        /* This block is shared between both loops */
        result ^= i;
        VOLATILE_DO(result);
        
        /* Do-while loop inside for loop */
        int m = 0;
        do {
            result += m * i;
            VOLATILE_DO(result);
            m++;
        } while (m < 3);
    }
    
    return result & 0xFF;
}

/* Function 4: Complex nested structure with while loop */
NOINLINE int complex_nested(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 35) + 5;
    
    /* Outer loop */
    int outer = 0;
    while (outer < limit) {
        VOLATILE_DO(outer);
        
        /* First middle loop */
        for (int mid = 0; mid < outer + 2; ++mid) {
            /* Innermost loop 1 */
            for (int inner = 0; inner < 4; ++inner) {
                int a = outer + mid + inner;
                int b = a * a;
                int c = b % 256;
                result ^= c;
                VOLATILE_DO(result);
            }
            
            /* Innermost loop 2 - different structure */
            int k = 0;
            while (k < 2) {
                result += (mid << k);
                VOLATILE_DO(result);
                k++;
            }
        }
        
        outer++;
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with break to shared label */
NOINLINE int adjacent_loops_break(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 25) + 10;
    
    /* Loop X */
    for (int x = 0; x < limit; ++x) {
        VOLATILE_DO(x);
        
        /* Loop Y - can break into Loop Z's body */
        for (int y = 0; y < 8; ++y) {
            if (x + y > limit) {
                goto shared_label;
            }
            result += x * y;
            VOLATILE_DO(result);
        }
        
        /* Loop Z */
        for (int z = 0; z < x + 1; ++z) {
        shared_label:
            int a = x + z;
            int b = a * 7;
            int c = b ^ result;
            result = c;
            VOLATILE_DO(result);
            
            if (z > 3) break;
        }
    }
    
    return result & 0xFF;
}

/* Function 6: Mixed loop types for varied CFG */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    volatile int seed = N;
    int limit = (seed % 20) + 8;
    
    /* For loop */
    for (int i = 0; i < limit; ++i) {
        VOLATILE_DO(i);
        
        /* Do-while inside for */
        int j = 0;
        do {
            result += i * j;
            VOLATILE_DO(result);
            j++;
        } while (j < 4);
        
        /* While loop after do-while */
        int k = 0;
        while (k < 3) {
            result ^= (i << k);
            VOLATILE_DO(result);
            k++;
        }
    }
    
    /* Separate while loop */
    int m = 0;
    while (m < limit / 2) {
        /* For loop inside while */
        for (int n = 0; n < m + 2; ++n) {
            result += m * n * 3;
            VOLATILE_DO(result);
        }
        m++;
    }
    
    return result & 0xFF;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int base = argc;
    int N = (base % 100) + 20;
    
    /* Call all test functions */
    total ^= perfect_nesting(N);
    total ^= loop_subset_of_other(N + 1);
    total ^= partial_overlap_goto(N + 2);
    total ^= complex_nested(N + 3);
    total ^= adjacent_loops_break(N + 4);
    total ^= mixed_loop_types(N + 5);
    
    /* Generate side effect to prevent elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
