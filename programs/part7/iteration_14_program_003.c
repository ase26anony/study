/* 
 * Test program for hardware loop optimization coverage.
 * Designed to trigger bitmap intersection logic in hw-doloop.cc lines 429-436.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * For generic targets: gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE_COLD __attribute__((noinline, cold))

/* Force hardware loop optimization on supported targets */
#ifdef __ARM_ARCH
#define HWLOOP_TARGET __attribute__((target("arch=armv8-a+lse")))
#else
#define HWLOOP_TARGET
#endif

/* Use volatile assembly to prevent optimization */
#define KEEP(i) asm volatile("" : : "r"(i))

/* Function 1: Perfect nesting - other is subset of loop */
/* Should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap) */
NOINLINE_COLD HWLOOP_TARGET
int perfect_nesting(int N, int M) {
    int result = 0;
    volatile int v = N; /* Prevent constant propagation */
    int n = v;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < n; ++i) {
        /* No code here to ensure inner loop blocks are subset of outer */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < M; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = c * i;
            int e = d ^ j;
            result += e;
            
            /* Prevent optimization */
            KEEP(a); KEEP(b); KEEP(c); KEEP(d); KEEP(e);
        }
        
        /* No code here either to maintain subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Loop is subset of other */
/* Should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap) */
NOINLINE_COLD HWLOOP_TARGET
int loop_subset_of_other(int N, int M) {
    int result = 0;
    volatile int v = N;
    int n = v;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 5; ++j) {
            int a = i * j;
            result ^= a;
            KEEP(a);
        }
        
        /* Some intermediate code in outer loop */
        int temp = i * 3;
        KEEP(temp);
        
        /* Second inner loop - this will be 'loop' (subset of 'other') */
        for (int k = 0; k < M; ++k) {
            /* Complex body for register pressure */
            int x = i + k;
            int y = x * x;
            int z = y - k;
            result += z;
            
            KEEP(x); KEEP(y); KEEP(z);
        }
        
        /* More code in outer loop after 'loop' */
        temp = result & 0xF;
        KEEP(temp);
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto */
/* Should trigger: bitmap_intersect_p is true, but both intersect_compl are true */
NOINLINE_COLD HWLOOP_TARGET
int overlapping_loops_goto(int N, int M) {
    int result = 0;
    volatile int v = N;
    int n = v;
    
    /* First loop - will be 'loop' */
    for (int i = 0; i < n; ++i) {
        int a = i * 2;
        
    shared_label:
        /* This block will be shared via goto */
        result += a;
        KEEP(a);
        
        /* Second loop - will be 'other' */
        for (int j = 0; j < M; ++j) {
            int b = i + j;
            result ^= b;
            KEEP(b);
            
            /* Jump into first loop's body */
            if (j == M/2 && i < n/2) {
                goto shared_label;
            }
        }
        
        /* Continue with first loop */
        a = result & 0xF;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE_COLD HWLOOP_TARGET
int mixed_loop_types(int N, int M) {
    int result = 0;
    volatile int v = N;
    int n = v;
    
    /* Outer for loop */
    for (int i = 0; i < n; ++i) {
        int count = M;
        
        /* Inner do-while loop */
        do {
            int a = i * count;
            int b = a << 2;
            int c = b ^ i;
            result += c;
            
            KEEP(a); KEEP(b); KEEP(c);
            
            count--;
        } while (count > 0);
        
        /* While loop after do-while */
        int temp = result;
        while (temp > 100) {
            temp >>= 1;
            result ^= temp;
            KEEP(temp);
        }
    }
    
    return result & 0xFF;
}

/* Function 5: Sibling loops with break to shared label */
NOINLINE_COLD HWLOOP_TARGET
int sibling_loops_break(int N, int M) {
    int result = 0;
    volatile int v = N;
    int n = v;
    
    /* First sibling loop */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < M; ++j) {
            if (j == 3) {
                goto shared_block;
            }
            result += i * j;
        }
        
        continue; /* This creates a basic block */
        
    shared_block:
        /* Shared block between loops */
        result ^= 0x55;
        
        /* Second sibling loop that can reach shared_block via break */
        for (int k = 0; k < 10; ++k) {
            result += k;
            if (k == 5) {
                break;
            }
        }
        
        /* This creates another block in first loop but not second */
        result &= 0xFF;
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create varying loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int M = (seed % 25) + 5;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1, M);
    total ^= loop_subset_of_other(N2, M);
    total ^= overlapping_loops_goto(N3, M);
    total ^= mixed_loop_types(N1, M/2);
    total ^= sibling_loops_break(N2, M);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
