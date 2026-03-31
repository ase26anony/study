/* test_loop_doloop.c
 * 
 * This program generates loops with decrement-and-compare-tail patterns
 * to trigger coverage in GCC's loop-doloop.cc lines 136-150.
 * The pattern matches: (set (reg:CC) (compare (plus (reg) -1) (const0)))
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loop bodies */
volatile int global_sink = 0;
int dummy_array[10000];

/* External function to prevent inlining */
void __attribute__((noinline)) side_effect(int value) {
    global_sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_int(int iterations) {
    volatile int local_sink = 0;
    int sum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        local_sink = i;          /* Volatile side effect */
        sum += i & 0xFF;         /* Simple computation */
        dummy_array[i & 0xFF] = i; /* Array store */
    }
    
    side_effect(sum);
    return sum;
}

/* Function B: while loop with unsigned int counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_unsigned(unsigned int n) {
    unsigned int sum = 0;
    volatile unsigned int counter = n;
    
    /* Decrement-and-compare in while condition */
    while (counter--) {
        sum += counter;
        global_sink = counter;   /* Global volatile side effect */
        dummy_array[counter % 1000] = (int)counter;
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    int total = 0;
    
    for (int j = 0; j < outer; j++) {
        volatile int inner_counter = inner;
        
        /* Inner loop with decrement-and-compare */
        while (inner_counter--) {
            total += j * inner_counter;
            side_effect(inner_counter);
        }
        
        /* Prevent outer loop optimization */
        global_sink = j;
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_loop(int count) {
    int result = 0;
    
    /* Use parameter directly in decrement pattern */
    for (; count-- > 0;) {
        result ^= count;         /* Non-trivial computation */
        dummy_array[count % 100] = result;
        global_sink = count;     /* Ensure side effect */
    }
    
    return result;
}

/* Function E: Do-while loop with explicit decrement (alternative pattern) */
int __attribute__((optimize("O2"))) test_dowhile_loop(int limit) {
    int i = limit;
    int acc = 0;
    
    if (i <= 0) return 0;
    
    do {
        acc += i;
        side_effect(i);
        dummy_array[i % 500] = i;
    } while (--i > 0);  /* Decrement and compare at end */
    
    return acc;
}

/* Function F: Loop with volatile bound (prevents compile-time folding) */
int __attribute__((optimize("O2"))) test_volatile_bound(void) {
    volatile int dynamic_bound = 1000;
    int sum = 0;
    int i = dynamic_bound;
    
    /* Volatile bound forces runtime evaluation */
    while (i--) {
        sum += i * 2;
        global_sink = i;
    }
    
    return sum;
}

/* Main driver that executes all tests and verifies results */
int main(void) {
    int checksum = 0;
    
    printf("Testing loop-doloop pattern coverage...\n");
    
    /* Test A: Basic int loop */
    int result_a = test_for_loop_int(1000);
    checksum += result_a;
    printf("Test A result: %d\n", result_a);
    
    /* Test B: Unsigned loop */
    unsigned int result_b = test_while_loop_unsigned(500);
    checksum += (int)result_b;
    printf("Test B result: %u\n", result_b);
    
    /* Test C: Nested loops */
    int result_c = test_nested_loops(10, 100);
    checksum += result_c;
    printf("Test C result: %d\n", result_c);
    
    /* Test D: Parameter loop */
    int result_d = test_param_loop(750);
    checksum += result_d;
    printf("Test D result: %d\n", result_d);
    
    /* Test E: Do-while loop */
    int result_e = test_dowhile_loop(300);
    checksum += result_e;
    printf("Test E result: %d\n", result_e);
    
    /* Test F: Volatile bound loop */
    int result_f = test_volatile_bound();
    checksum += result_f;
    printf("Test F result: %d\n", result_f);
    
    printf("Final checksum: %d\n", checksum);
    printf("Global side effects: %d\n", global_sink);
    
    /* Verify array was touched */
    int array_check = 0;
    for (int i = 0; i < 100; i++) {
        array_check += dummy_array[i];
    }
    printf("Array checksum: %d\n", array_check);
    
    return 0;
}
