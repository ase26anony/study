/* test_loop_doloop.c
 * 
 * This program is designed to trigger coverage of the decrement-and-compare
 * pattern matching logic in GCC's loop-doloop.cc pass (lines 136-150).
 * Each test function contains a loop that should compile to the RTL pattern:
 *   (set (reg:CC) (compare (plus (reg) -1) (const0)))
 */

#include <stdio.h>
#include <stdlib.h>

/* Global volatile variable to prevent loop elimination */
volatile int global_sink = 0;

/* External array to create side effects */
extern int extern_array[10000];

/* Non-inline dummy function to prevent optimization */
static void __attribute__((noinline)) dummy_side_effect(int val) {
    global_sink += val;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int limit) {
    int sum = 0;
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = limit; i-- > 0;) {
        sum += i;
        dummy_side_effect(i);
    }
    return sum;
}

/* Function B: while loop with unsigned int counter */
int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    int sum = 0;
    /* Decrement-and-compare in while condition */
    while (n--) {
        sum += n;
        extern_array[n % 1000] = n;  /* External side effect */
    }
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        int i = inner;
        while (i--) {
            total += o * i;
            dummy_side_effect(i);
        }
    }
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    int result = 0;
    /* Use volatile to hide loop bound from optimizer */
    volatile int vol_count = count;
    int j = vol_count;
    
    /* Decrement-and-compare with parameter counter */
    do {
        result += j;
        extern_array[j % 1000] = result;
    } while (j-- > 0);
    
    return result;
}

/* Function E: Multiple decrement-and-compare loops in same function */
int __attribute__((optimize("O2"))) test_multiple_loops(int a, int b) {
    int checksum = 0;
    
    /* First loop */
    for (int i = a; i-- > 0;) {
        checksum ^= i;
        dummy_side_effect(i);
    }
    
    /* Second loop with different counter */
    unsigned int u = b;
    while (u--) {
        checksum += u;
        extern_array[u % 1000] = checksum;
    }
    
    return checksum;
}

/* Helper to initialize external array */
void init_array(void) {
    for (int i = 0; i < 1000; i++) {
        extern_array[i] = 0;
    }
}

/* Main driver that runs all tests and computes final checksum */
int main(void) {
    init_array();
    
    int final_checksum = 0;
    
    /* Test A: Basic int loop */
    int res_a = test_for_loop_int(1000);
    final_checksum += res_a;
    printf("Test A result: %d\n", res_a);
    
    /* Test B: Unsigned while loop */
    int res_b = test_while_loop_unsigned(500);
    final_checksum += res_b;
    printf("Test B result: %d\n", res_b);
    
    /* Test C: Nested loops */
    int res_c = test_nested_loops(10, 100);
    final_checksum += res_c;
    printf("Test C result: %d\n", res_c);
    
    /* Test D: Parameter counter */
    int res_d = test_param_counter(300);
    final_checksum += res_d;
    printf("Test D result: %d\n", res_d);
    
    /* Test E: Multiple loops */
    int res_e = test_multiple_loops(200, 150);
    final_checksum += res_e;
    printf("Test E result: %d\n", res_e);
    
    /* Add global sink to prevent dead code elimination */
    final_checksum += global_sink;
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Verification: ensure loops actually executed */
    if (final_checksum != 0) {
        printf("SUCCESS: All loops executed (non-zero checksum)\n");
    }
    
    return 0;
}

/* Define the external array */
int extern_array[10000] = {0};
