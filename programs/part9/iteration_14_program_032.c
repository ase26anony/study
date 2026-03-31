/* test_hwloop.c
 * 
 * This test is designed for targets with hardware loop support.
 * Compile with: gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a
 * 
 * The program creates complex loop structures to trigger bitmap intersection
 * logic in GCC's hardware loop optimization pass (hw-doloop.cc).
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Force specific optimization level */
#define OPTIMIZE_O2 __attribute__((optimize("O2")))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Create register pressure with multiple variables */
#define CREATE_REG_PRESSURE(i) \
    int a = (i) ^ 0x55; \
    int b = a * 3; \
    int c = b - (i); \
    int d = c ^ a; \
    int e = d * 7; \
    result = (result ^ e) + b; \
    asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(e))

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: loop->loops.safe_push(other)
 */
NOINLINE OPTIMIZE_O2
int perfect_nesting(int n) {
    int result = 0;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < n; ++i) {
        /* No code here to ensure other is pure subset */
        
        /* Inner loop - this will be 'other' */
        for (int j = 0; j < i + 1; ++j) {
            CREATE_REG_PRESSURE(j);
        }
        
        /* No code here either */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE OPTIMIZE_O2
int reverse_nesting(int n) {
    int result = 0;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            int temp = i * j;
            result ^= temp;
            asm volatile("" : : "r"(temp));
        }
        
        /* Second inner loop - this will be 'loop' (subset of other) */
        for (int k = 0; k < i + 1; ++k) {
            CREATE_REG_PRESSURE(k);
        }
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * Creates intersection but neither is subset
 */
NOINLINE OPTIMIZE_O2
int overlapping_loops(int n) {
    int result = 0;
    
    /* Loop A */
    for (int i = 0; i < n; ++i) {
        CREATE_REG_PRESSURE(i);
        
        if (i == n/2) {
            /* Jump into Loop B's body */
            goto inside_loop_b;
        }
    }
    
    /* Loop B */
    for (int j = 0; j < n; ++j) {
        inside_loop_b:
        CREATE_REG_PRESSURE(j);
        
        /* Mix loop types: do-while inside for */
        int m = 3;
        do {
            result ^= m;
            m--;
        } while (m > 0);
    }
    
    return result & 0xFF;
}

/* Function 4: Complex nested structure with while loop */
NOINLINE OPTIMIZE_O2
int mixed_loop_types(int n) {
    int result = 0;
    
    /* Outer for loop */
    for (int i = 0; i < n; ++i) {
        /* Inner while loop */
        int w = i;
        while (w > 0) {
            CREATE_REG_PRESSURE(w);
            w /= 2;
        }
        
        /* Another for loop as sibling */
        for (int k = 0; k < 5; ++k) {
            result += k * i;
        }
    }
    
    /* Follow with a separate while loop */
    int count = n;
    while (count-- > 0) {
        result ^= count;
    }
    
    return result & 0xFF;
}

/* Function 5: Disjoint loops with shared label via switch */
NOINLINE OPTIMIZE_O2
int switch_between_loops(int n) {
    int result = 0;
    
    for (int i = 0; i < n; ++i) {
        switch (i % 3) {
            case 0:
                /* Jump target shared between loops */
                shared_label:
                CREATE_REG_PRESSURE(i);
                break;
            case 1:
                /* Another loop that can jump to shared_label */
                for (int j = 0; j < 2; ++j) {
                    if (j == 1) goto shared_label;
                    result += j;
                }
                break;
            default:
                /* Third loop structure */
                do {
                    result ^= i;
                    i++;
                } while (i % 5 != 0);
                break;
        }
    }
    
    return result & 0xFF;
}

/* Main driver that calls all functions */
int main(int argc, char *argv[]) {
    /* Use volatile to prevent constant propagation */
    volatile int seed = argc > 1 ? atoi(argv[1]) : global_seed;
    int N = (seed % 100) + 10;
    
    int total = 0;
    
    /* Call each function multiple times with different parameters */
    total ^= perfect_nesting(N);
    total ^= reverse_nesting(N + 1);
    total ^= overlapping_loops(N + 2);
    total ^= mixed_loop_types(N + 3);
    total ^= switch_between_loops(N + 4);
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
