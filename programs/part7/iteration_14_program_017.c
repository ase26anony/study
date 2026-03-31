/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * Or for generic target: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Function 1: Perfect nesting - other is subset of loop (Condition 2) */
NOINLINE int perfect_nesting(int N) {
    volatile int result = 0;
    int a, b, c, d, e, f; /* Create register pressure */
    
    /* Outer loop - this will be 'loop' in the hierarchy */
    for (int i = 0; i < N; ++i) {
        a = i * 2;
        b = i + 1;
        
        /* Inner loop - this will be 'other' that is subset of outer */
        for (int j = 0; j < (i % 5) + 1; ++j) {
            c = a * j;
            d = b - j;
            e = c ^ d;
            f = e << 2;
            result += f;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(c), "r"(d), "r"(e), "r"(f));
        }
        
        /* No code here ensures inner loop is perfect subset */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other (Condition 3) */
NOINLINE int loop_subset_of_other(int N) {
    volatile int result = 0;
    int x1, x2, x3, x4, x5;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        x1 = i * 3;
        x2 = x1 ^ 0x55;
        
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            x3 = x2 + j;
            result ^= x3;
            asm volatile("" : : "r"(x3));
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (i % 3) + 1; ++k) {
            x4 = x1 * k;
            x5 = x4 >> 1;
            result += x5;
            asm volatile("" : : "r"(x4), "r"(x5));
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partial overlap with goto (Condition 1) */
NOINLINE int partial_overlap_goto(int N) {
    volatile int result = 0;
    int v1, v2, v3;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        v1 = i * i;
        
    loop_a_body:
        v2 = v1 + i;
        
        /* Loop B - shares block via goto */
        for (int j = 0; j < 3; ++j) {
            v3 = v2 * j;
            result += v3;
            
            if (j == 1 && (result & 1)) {
                /* Jump into Loop A's body, creating intersection */
                goto loop_a_body;
            }
            
            asm volatile("" : : "r"(v3));
        }
        
        result ^= v2;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with complex nesting */
NOINLINE int mixed_loop_types(int N) {
    volatile int result = 0;
    int t1, t2, t3, t4;
    
    /* while loop */
    int w = 0;
    while (w < N) {
        t1 = w * 2;
        
        /* do-while nested inside while */
        int dw = 0;
        do {
            t2 = t1 + dw;
            result += t2;
            asm volatile("" : : "r"(t2));
            dw++;
        } while (dw < 3);
        
        /* for loop after do-while (sibling relationship) */
        for (int f = 0; f < 2; ++f) {
            t3 = t1 * f;
            t4 = t3 ^ result;
            result = t4;
            asm volatile("" : : "r"(t3), "r"(t4));
        }
        
        w++;
    }
    
    return result & 0xFF;
}

/* Function 5: Adjacent loops with shared basic block via switch */
NOINLINE int adjacent_loops_switch(int N) {
    volatile int result = 0;
    int s1, s2, s3;
    
    /* First loop */
    for (int i = 0; i < N; ++i) {
        s1 = i % 4;
        
        switch(s1) {
        case 0:
            s2 = i * 2;
            break;
        case 1:
            s2 = i + 5;
            break;
        default:
            s2 = i - 3;
        }
        
        result += s2;
    }
    
    /* Second loop - shares the switch block */
    for (int j = N; j > 0; --j) {
        s1 = j % 4;
        
        /* Same switch structure - creates shared basic block */
        switch(s1) {
        case 0:
            s3 = j * 3;
            break;
        case 1:
            s3 = j + 7;
            break;
        default:
            s3 = j - 2;
        }
        
        result ^= s3;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent constant propagation */
int main(int argc, char *argv[]) {
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 20) + 25;
    int N5 = (seed % 10) + 30;
    
    int total = 0;
    
    /* Call all functions to ensure they're compiled and executed */
    total += perfect_nesting(N1);
    total += loop_subset_of_other(N2);
    total += partial_overlap_goto(N3);
    total += mixed_loop_types(N4);
    total += adjacent_loops_switch(N5);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
