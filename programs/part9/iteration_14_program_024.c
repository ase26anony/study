/* test_hwloop.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Compilation for hardware loop targets (e.g., ARMv8):
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 *   gcc -fprofile-arcs -ftest-coverage test_hwloop.o -o test_hwloop_executable
 * 
 * For aggressive loop optimization:
 *   gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage -march=armv8-a test_hwloop.c -o test_hwloop_executable
 */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure separate function compilation */
#define NOINLINE __attribute__((noinline, cold))

/* Use volatile to prevent constant propagation */
static volatile int global_seed = 42;

/* Function 1: Perfect nesting - other is subset of loop
 * This should trigger: !bitmap_intersect_compl_p(other->block_bitmap, loop->block_bitmap)
 * Result: loop->loops.safe_push(other)
 */
NOINLINE
int perfect_nesting(int n) {
    int result = 0;
    int i, j;
    
    /* Outer loop (will be 'loop' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* No code here - ensures outer loop blocks are superset of inner */
        
        /* Inner loop (will be 'other' in hierarchy) */
        for (j = 0; j < i; ++j) {
            /* Create register pressure */
            int a = i + j;
            int b = a * 2;
            int c = b - a;
            int d = (a * b) >> (c & 3);
            result ^= d;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c), "r"(d));
        }
        
        /* No code here either - maintains subset relationship */
    }
    
    return result & 0xFF;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: !bitmap_intersect_compl_p(loop->block_bitmap, other->block_bitmap)
 * Result: other->loops.safe_push(loop)
 */
NOINLINE
int reverse_nesting(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop (will be 'other' in hierarchy) */
    for (i = 0; i < n; ++i) {
        /* First inner loop - creates blocks in 'other' not in 'loop' */
        for (j = 0; j < 3; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Second inner loop (will be 'loop' in hierarchy) */
        for (k = 0; k < i; ++k) {
            /* This loop is subset of outer loop blocks */
            int b = i + k;
            int c = b * 3;
            int d = c - b;
            result ^= (b * c) >> (d & 3);
            asm volatile("" : : "r"(b), "r"(c), "r"(d));
        }
        
        /* More code in outer loop after 'loop' */
        result += i * 7;
    }
    
    return result & 0xFF;
}

/* Function 3: Partially overlapping loops with goto
 * This should trigger bitmap intersection but not subset relationship
 * The first 'if' condition will be true, but not the second or third
 */
NOINLINE
int overlapping_loops(int n) {
    int result = 0;
    int i = 0, j = 0;
    
    /* First loop (could be 'loop' or 'other') */
    while (i < n) {
        int a = i * 2;
        result += a;
        
        if (i == n/2) {
            /* Jump into second loop's body */
            goto inside_second_loop;
        }
        
        i++;
    }
    
    /* Second loop */
    for (j = 0; j < n; ++j) {
        inside_second_loop:
        int b = j * 3;
        int c = b - j;
        result ^= c;
        asm volatile("" : : "r"(b), "r"(c));
        
        if (j > n/2) {
            /* Jump back to first loop */
            i = j;
            break;
        }
    }
    
    /* Continue first loop */
    while (i < n) {
        int d = i * 5;
        result += d;
        i++;
    }
    
    return result & 0xFF;
}

/* Function 4: Mixed loop types with do-while inside for */
NOINLINE
int mixed_loops(int n) {
    int result = 0;
    int i, j;
    
    /* Outer for loop */
    for (i = 0; i < n; ++i) {
        /* Inner do-while loop */
        j = 0;
        do {
            int a = i + j;
            int b = a * a;
            int c = b % 256;
            result = (result + c) & 0xFF;
            
            /* Complex expression for register pressure */
            int d = (a << 3) | (b >> 5);
            int e = d ^ c;
            int f = e * i;
            asm volatile("" : : "r"(d), "r"(e), "r"(f));
            
            j++;
        } while (j < 5);
        
        /* Code between loops in outer loop */
        result ^= i * 11;
    }
    
    /* Following while loop */
    int k = n;
    while (k > 0) {
        result += k;
        k--;
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE
int sibling_loops(int n) {
    int result = 0;
    int i, j, k;
    
    /* Outer loop */
    for (i = 0; i < n; ++i) {
        /* First sibling inner loop */
        for (j = 0; j < i; ++j) {
            int a = i * j;
            result += a;
            asm volatile("" : : "r"(a));
        }
        
        /* Code between sibling loops */
        result ^= i * 13;
        
        /* Second sibling inner loop */
        for (k = 0; k < n - i; ++k) {
            int b = i + k;
            int c = b * 7;
            result ^= c;
            asm volatile("" : : "r"(b), "r"(c));
        }
        
        /* Third loop inside outer but after siblings */
        int m = 0;
        while (m < 2) {
            result += m * i;
            m++;
        }
    }
    
    return result & 0xFF;
}

/* Main function with volatile inputs to prevent optimization */
int main(int argc, char *argv[]) {
    int total = 0;
    
    /* Use volatile and argc to create variable loop bounds */
    volatile int seed = argc;
    int N1 = (seed % 50) + 10;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 20;
    int N4 = (seed % 60) + 5;
    int N5 = (seed % 35) + 25;
    
    /* Call all functions to ensure they're compiled and executed */
    total ^= perfect_nesting(N1);
    total ^= reverse_nesting(N2);
    total ^= overlapping_loops(N3);
    total ^= mixed_loops(N4);
    total ^= sibling_loops(N5);
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total & 0xFF);
    
    return 0;
}
