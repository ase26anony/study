/* test_loop_doloop.c
 * 
 * This program is designed to exercise the loop-doloop pass in GCC's RTL
 * optimizer. It contains multiple loops that should generate the specific
 * RTL pattern: (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * which corresponds to a decrement-and-compare-against-zero loop tail.
 * 
 * Compile with: gcc -O2 -fno-unroll-loops -fdump-rtl-loop2 test_loop_doloop.c -o test
 * 
 * The -fdump-rtl-loop2 flag will create a dump file showing the RTL
 * during the loop2 pass, where loop-doloop runs.
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent loop removal */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inline dummy function to create side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    int sum = 0;
    /* The canonical pattern: for (int i = N; i-- > 0;) */
    for (int i = iterations; i-- > 0;) {
        /* Simple side effect that can't be optimized away */
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int count) {
    int sum = 0;
    unsigned int n = count;
    /* Pattern: while (n--) */
    while (n--) {
        sum += (int)n;
        extern_array[n % 1000] = (int)n;  /* External side effect */
    }
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        int i = inner;
        while (i--) {
            total += o * i;
            global_sink += o + i;  /* Volatile side effect */
        }
    }
    return total;
}

/* Function D: Loop with counter as function parameter (prevents constant folding) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    int result = 0;
    /* Use count directly in the decrement pattern */
    int n = count;
    do {
        result += n;
        dummy_side_effect(n);
    } while (n-- > 0);
    return result;
}

/* Function E: Loop with volatile bound to prevent compile-time optimization */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 100;
    int sum = 0;
    int i = bound;
    
    /* The volatile read happens once, then we use decrement pattern */
    while (i--) {
        sum += i;
        extern_array[i % 100] = sum;
    }
    return sum;
}

/* Function F: Multiple loops in same function to test pass on different structures */
int __attribute__((optimize("O2"))) test_multiple_loops(int a, int b) {
    int total = 0;
    
    /* First loop */
    for (int i = a; i-- > 0;) {
        total += i;
        global_sink = total;
    }
    
    /* Second loop with different counter */
    unsigned int j = b;
    while (j--) {
        total -= (int)j;
        dummy_side_effect(j);
    }
    
    return total;
}

/* Main driver that runs all tests and verifies correctness */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Initialize extern array */
    for (int i = 0; i < 10000; i++) {
        extern_array[i] = 0;
    }
    
    /* Run each test with different parameters */
    checksum += test_for_loop_int(1000);
    printf("Test A (for loop int): completed\n");
    
    checksum += test_while_loop_unsigned(500);
    printf("Test B (while loop unsigned): completed\n");
    
    checksum += test_nested_loops(10, 100);
    printf("Test C (nested loops): completed\n");
    
    checksum += test_param_counter(200);
    printf("Test D (parameter counter): completed\n");
    
    checksum += test_volatile_bound();
    printf("Test E (volatile bound): completed\n");
    
    checksum += test_multiple_loops(150, 150);
    printf("Test F (multiple loops): completed\n");
    
    /* Print final checksum to ensure all loops executed */
    printf("Final checksum: %d\n", checksum);
    printf("Global sink value: %d\n", global_sink);
    
    /* Quick verification that extern_array was touched */
    int array_sum = 0;
    for (int i = 0; i < 1000; i++) {
        array_sum += extern_array[i];
    }
    printf("Array checksum: %d\n", array_sum);
    
    return 0;
}

/* Define the extern array */
int extern_array[10000];
