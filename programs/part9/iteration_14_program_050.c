/* test_hwloop.c
 * 
 * This test is designed to trigger specific bitmap intersection logic
 * in GCC's hardware loop optimization pass (hw-doloop.cc).
 * 
 * Compilation for hardware loop targets (e.g., ARMv8):
 *   gcc -O2 -doloop -fprofile-arcs -ftest-coverage -march=armv8-a -c test_hwloop.c -o test_hwloop.o
 *   gcc -fprofile-arcs test_hwloop.o -o test_hwloop_executable
 * 
 * For aggressive loop optimization:
 *   gcc -O3 -funroll-loops -fpeel-loops -fprofile-arcs -ftest-coverage test_hwloop.c -o test_hwloop_executable -march=native
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
NOINLINE
int perfect_nesting(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer loop - this will be 'loop' */
    for (int i = 0; i < N; ++i) {
        /* Inner loop - this will be 'other' (subset of outer) */
        for (int j = 0; j < (i % 5) + 1; ++j) {
            /* Create register pressure */
            a = i * j;
            b = a + j;
            c = b - i;
            
            /* Prevent optimization */
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            
            result ^= (a * b) >> (c & 7);
        }
        
        /* Small amount of code in outer loop but not in inner */
        result += i & 1;
    }
    
    return result;
}

/* Function 2: Reverse nesting - loop is subset of other
 * This should trigger: other->loops.safe_push(loop)
 */
NOINLINE
int reverse_nesting(int N) {
    int result = 0;
    int x, y, z;
    
    /* Outer loop - this will be 'other' */
    for (int i = 0; i < N; ++i) {
        /* First inner loop - creates blocks in 'other' but not in 'loop' */
        for (int j = 0; j < 2; ++j) {
            x = i + j;
            y = x * 3;
            z = y - j;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result += x ^ y ^ z;
        }
        
        /* Second inner loop - this will be 'loop' (subset of outer) */
        for (int k = 0; k < (i % 3) + 1; ++k) {
            x = i * k;
            y = x + 7;
            z = y - k;
            asm volatile("" : : "r"(x), "r"(y), "r"(z));
            result ^= (x * y) >> (z & 3);
        }
        
        /* More code in outer loop but not in 'loop' */
        result -= i & 3;
    }
    
    return result;
}

/* Function 3: Partially overlapping loops with goto
 * This should trigger the first condition (bitmap_intersect_p is true)
 * but not the subset conditions
 */
NOINLINE
int overlapping_loops(int N) {
    int result = 0;
    int p, q, r;
    
    /* Loop A */
    for (int i = 0; i < N; ++i) {
        p = i * 2;
        q = p + 1;
        
    shared_label:
        r = q - i;
        asm volatile("" : : "r"(p), "r"(q), "r"(r));
        result += p ^ q;
        
        /* Loop B - shares the block at shared_label via goto */
        for (int j = 0; j < (i % 4) + 1; ++j) {
            if (j == 1) {
                goto shared_label;  /* Jump into Loop A's body */
            }
            p = j * 3;
            q = p + j;
            result ^= q;
        }
        
        result += r;
    }
    
    return result;
}

/* Function 4: Mixed loop types (do-while inside for) */
NOINLINE
int mixed_loops(int N) {
    int result = 0;
    int a, b, c;
    
    /* Outer for loop */
    for (int i = 0; i < N; ++i) {
        int counter = (i % 3) + 2;
        
        /* Inner do-while loop */
        do {
            a = counter * i;
            b = a + counter;
            c = b - i;
            asm volatile("" : : "r"(a), "r"(b), "r"(c));
            result ^= (a + b) * c;
            
            counter--;
        } while (counter > 0);
        
        /* While loop after do-while */
        int w = 0;
        while (w < 2) {
            a = i + w;
            b = a * w;
            asm volatile("" : : "r"(a), "r"(b));
            result += b;
            w++;
        }
    }
    
    return result;
}

/* Function 5: Complex nested structure with sibling loops */
NOINLINE
int complex_siblings(int N) {
    int result = 0;
    int tmp1, tmp2, tmp3;
    
    /* Outer loop */
    for (int i = 0; i < N; ++i) {
        /* First sibling inner loop */
        for (int j = 0; j < 3; ++j) {
            tmp1 = i * j;
            tmp2 = tmp1 + 5;
            asm volatile("" : : "r"(tmp1), "r"(tmp2));
            result += tmp1 ^ tmp2;
        }
        
        /* Code between sibling loops (in outer but not in either inner) */
        result ^= i << 2;
        
        /* Second sibling inner loop */
        for (int k = 0; k < 2; ++k) {
            tmp1 = i + k;
            tmp2 = tmp1 * 7;
            tmp3 = tmp2 - k;
            asm volatile("" : : "r"(tmp1), "r"(tmp2), "r"(tmp3));
            result ^= tmp3;
        }
        
        /* Another loop at same nesting level */
        int m = 0;
        while (m < (i % 2) + 1) {
            tmp1 = m * i;
            asm volatile("" : : "r"(tmp1));
            result -= tmp1;
            m++;
        }
    }
    
    return result;
}

/* Main function to drive all test cases */
int main(int argc, char **argv) {
    int total_result = 0;
    
    /* Use volatile and argc to prevent constant propagation */
    volatile int seed = argc;
    int N1 = (seed % 50) + 20;
    int N2 = (seed % 40) + 15;
    int N3 = (seed % 30) + 10;
    int N4 = (seed % 35) + 12;
    int N5 = (seed % 45) + 18;
    
    /* Call all test functions */
    total_result ^= perfect_nesting(N1);
    total_result ^= reverse_nesting(N2);
    total_result ^= overlapping_loops(N3);
    total_result ^= mixed_loops(N4);
    total_result ^= complex_siblings(N5);
    
    /* Generate side effect to prevent elimination */
    printf("Result: %d\n", total_result & 255);
    
    return 0;
}
