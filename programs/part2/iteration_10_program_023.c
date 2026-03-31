/* loop-doloop-coverage.c
 * 
 * This program generates loops that compile to the specific RTL pattern:
 * (set (reg:CC) (compare (plus (reg) -1) (const0)))
 * which is matched by GCC's loop-doloop pass.
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int g_volatile_counter = 0;
int g_global_array[10000];
extern int g_external; /* Defined elsewhere to prevent constant propagation */

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    g_volatile_counter += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* The key pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        sink = i;  /* Volatile store prevents dead code elimination */
        sum += i;
        dummy_side_effect(i); /* External call prevents optimization */
    }
    
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int sink = 0;
    unsigned int sum = 0;
    
    /* while(n--) pattern */
    while (n--) {
        sink = n;
        sum += n;
        g_global_array[n % 1000] = n; /* Array store with side effect */
    }
    
    return sum;
}

/* Function C: Nested loops where inner loop uses the pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop with decrement-and-compare pattern */
        for (int i = inner; i-- > 0;) {
            sink = o * i;
            total += o * i;
            dummy_side_effect(o + i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    volatile int sink = 0;
    int result = 0;
    
    /* Use parameter directly in loop condition */
    while (count--) {
        sink = count;
        result ^= count; /* Non-linear operation prevents simplification */
        g_global_array[count % 1000] = result;
    }
    
    return result;
}

/* Function E: Do-while loop that should also match the pattern */
int __attribute__((optimize("O2"))) test_dowhile_loop(int iterations) {
    volatile int sink = 0;
    int i = iterations;
    int sum = 0;
    
    if (i <= 0) return 0;
    
    do {
        sink = i;
        sum += i;
        dummy_side_effect(i);
    } while (i-- > 1); /* Decrement and compare at the end */
    
    return sum;
}

/* Function F: Loop with volatile bound (prevents compile-time evaluation) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int bound = 100;
    volatile int sink = 0;
    int i = bound;
    int sum = 0;
    
    /* Volatile read prevents bound from being constant */
    while (i--) {
        sink = i;
        sum += i;
        g_global_array[i % 1000] = i;
    }
    
    return sum;
}

/* Function G: Multiple decrement patterns in same function */
int __attribute__((optimize("O2"))) test_multiple_patterns(int a, int b) {
    volatile int sink = 0;
    int result = 0;
    
    /* First loop */
    for (int i = a; i-- > 0;) {
        sink = i;
        result += i;
    }
    
    /* Second loop */
    int j = b;
    while (j--) {
        sink = j;
        result += j * 2;
    }
    
    return result;
}

/* Main driver that ensures all loops execute */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test with various iteration counts to exercise different paths */
    checksum += test_for_loop_decrement(1000);
    checksum += test_while_loop_decrement(500);
    checksum += test_nested_loops(10, 100);
    checksum += test_param_loop(300);
    checksum += test_dowhile_loop(200);
    checksum += test_volatile_bound();
    checksum += test_multiple_patterns(150, 150);
    
    /* Add some external dependency to prevent dead code elimination */
    if (&g_external) {
        checksum += g_volatile_counter;
    }
    
    printf("Final checksum: %d\n", checksum);
    printf("If this prints, all loops executed.\n");
    
    return 0;
}

/* External variable definition to satisfy linker */
int g_external = 42;
