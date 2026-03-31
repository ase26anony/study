/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures with specific block relationships
 * to trigger bitmap intersection logic in GCC's hardware loop optimization.
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop */
NOINLINE int perfect_nesting(int N) {
    int result = 0;
    volatile int keep = N; /* Prevent optimization */
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* No blocks here - inner loop starts immediately */
        
        /* Inner loop (will be 'other' - subset of outer) */
        for (int j = 0; j < (keep % 5) + 1; ++j) {
            /* Create register pressure */
            int a = i * j;
            int b = a + i;
            int c = b - j;
            int d = c * 3;
            int e = d >> 2;
            
            /* Prevent dead code elimination */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e));
            result ^= (a + b + c + d + e) & 0xFF;
        }
        
        /* No blocks here - outer loop ends after inner */
    }
    
    return result;
}

/* Function 2: Loop is subset of other */
NOINLINE int loop_subset_of_other(int N) {
    int result = 0;
    volatile int keep = N;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int x = i * j * 3;
            asm volatile("" : : "r"(x));
            result += x;
        }
        
        /* Some intermediate code in 'other' but not in 'loop' */
        int temp = i * 7;
        asm volatile("" : : "r"(temp));
        
        /* Second inner loop (will be 'loop' - subset of 'other') */
        for (int k = 0; k < (keep % 3) + 1; ++k) {
            int a = i + k;
            int b = a * 2;
            int c = b - k;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result ^= (a * b) >> (c & 3);
        }
        
        /* More code in 'other' but not in 'loop' */
        temp = i * 11;
        asm volatile("" : : "r"(temp));
        result += temp & 0xF;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops using goto */
NOINLINE int overlapping_loops_goto(int N) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        int a = i * 3;
        
        /* Loop B - partially overlaps with A via goto */
        for (int j = 0; j < 3; ++j) {
            int b = a + j;
            
            if (b % 5 == 0) {
                /* Jump into Loop A's body */
                goto inside_loop_a;
            }
            
            result += b;
            continue;
            
        inside_loop_a:
            /* This label is inside Loop A but reachable from Loop B */
            int c = b * 7;
            asm volatile("" : : "r"(c));
            result ^= c;
            break; /* Break from Loop B but stay in Loop A */
        }
        
        /* Continue Loop A */
        result += i;
    }
    
    return result;
}

/* Function 4: Mixed loop types with complex relationships */
NOINLINE int mixed_loop_types(int N) {
    int result = 0;
    int i = 0;
    
    /* while loop */
    while (i < N) {
        /* do-while nested inside while */
        int j = 0;
        do {
            int a = i * j;
            int b = (a << 2) | (a >> 6);
            asm volatile("" : : "r"(a), "r"(b));
            result += b & 0xFF;
            j++;
        } while (j < 3);
        
        /* for loop after do-while */
        for (int k = 0; k < 2; k++) {
            int c = i + k;
            int d = c * c - k;
            asm volatile("" : : "r"(c), "r"(d));
            result ^= d;
        }
        
        i++;
    }
    
    return result;
}

/* Function 5: Sibling loops with shared block via break */
NOINLINE int sibling_loops_shared_block(int N) {
    int result = 0;
    
    /* First loop */
    for (int i = 0; i < N; i++) {
        if (i % 2 == 0) {
            shared_block:
            /* This block is shared between both loops */
            int x = i * 13;
            asm volatile("" : : "r"(x));
            result += x & 0xF;
            continue;
        }
        
        result += i;
    }
    
    /* Second loop - shares the 'shared_block' via goto */
    for (int j = N - 1; j >= 0; j--) {
        if (j % 3 == 0) {
            goto shared_block;
        }
        
        int y = j * 17;
        asm volatile("" : : "r"(y));
        result ^= y;
    }
    
    return result;
}

/* Function 6: Complex nested structure with multiple levels */
NOINLINE int complex_multi_level(int N) {
    int result = 0;
    
    /* Level 1 */
    for (int a = 0; a < N; a++) {
        /* Level 2 - first inner */
        for (int b = 0; b < 2; b++) {
            /* Level 3 */
            for (int c = 0; c < 2; c++) {
                int t1 = a * b * c;
                asm volatile("" : : "r"(t1));
                result += t1;
            }
        }
        
        /* Level 2 - second inner (sibling of first) */
        for (int d = 0; d < 3; d++) {
            int t2 = a * d * 5;
            asm volatile("" : : "r"(t2));
            result ^= t2;
        }
        
        /* Level 2 - third inner with early exit */
        int e = 0;
        while (e < 4) {
            if ((a + e) % 7 == 0) break;
            int t3 = a * e * 11;
            asm volatile("" : : "r"(t3));
            result += t3;
            e++;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    int N6 = (seed % 5) + 35;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= loop_subset_of_other(N2);
    total ^= overlapping_loops_goto(N3);
    total ^= mixed_loop_types(N4);
    total ^= sibling_loops_shared_block(N5);
    total ^= complex_multi_level(N6);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return total & 0xFF;
}
