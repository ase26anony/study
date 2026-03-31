/* loop-doloop-coverage.c
 * Test program to cover decrement-and-compare pattern matching in GCC's loop-doloop pass
 */

#include <stdio.h>
#include <stdint.h>

/* Prevent optimization of loops */
volatile int global_volatile = 0;
int global_array[10000];

/* Dummy function with side effects to prevent loop elimination */
__attribute__((noinline)) 
void dummy_side_effect(int value) {
    global_volatile += value;
}

/* Function A: Basic for loop with int counter using post-decrement pattern */
__attribute__((optimize("O2")))
int test_basic_for_loop(int iterations) {
    volatile int sink = 0;
    int sum = 0;
    
    /* This should generate: (compare (plus (reg) -1) (const0)) */
    for (int i = iterations; i-- > 0;) {
        sink += i;               /* Volatile-like side effect */
        sum += i & 0xFF;         /* Prevent dead code elimination */
        dummy_side_effect(i);    /* External side effect */
    }
    
    return sum;
}

/* Function B: while loop with unsigned int counter */
__attribute__((optimize("O2")))
int test_while_loop(unsigned int count) {
    volatile int sink = 0;
    int sum = 0;
    unsigned int n = count;
    
    /* while(n--) pattern should generate the target RTL */
    while (n--) {
        sink += n;
        sum += (int)n % 256;
        global_array[n % 1000] = n;  /* Array store for side effect */
    }
    
    return sum;
}

/* Function C: Nested loops with inner loop using the pattern */
__attribute__((optimize("O2")))
int test_nested_loops(int outer_iter, int inner_iter) {
    volatile int sink = 0;
    int total = 0;
    
    for (int o = 0; o < outer_iter; o++) {
        int inner = inner_iter;
        
        /* Inner loop with decrement-and-compare tail */
        while (inner--) {
            sink += o * inner;
            total += (o + inner) & 0xFF;
            dummy_side_effect(inner);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
__attribute__((optimize("O2")))
int test_param_counter(int count) {
    volatile int sink = 0;
    int result = 0;
    int i = count;
    
    /* Use parameter as counter to prevent compile-time folding */
    do {
        sink += i;
        result += i * 3;
        global_volatile++;  /* Global side effect */
    } while (i-- > 0);      /* do-while with post-decrement */
    
    return result;
}

/* Function E: Mixed counter types to test different code generation */
__attribute__((optimize("O2")))
int test_mixed_counters(short short_count, char char_count) {
    volatile int sink = 0;
    int sum = 0;
    
    /* Test with short counter */
    for (short s = short_count; s-- > 0;) {
        sink += s;
        sum += s * 2;
    }
    
    /* Test with char counter */
    unsigned char c = char_count;
    while (c--) {
        sink += c;
        sum += c * 3;
        global_array[c % 100] = c;
    }
    
    return sum;
}

/* Function F: Loop with volatile bound to prevent optimization */
__attribute__((optimize("O2")))
int test_volatile_bound(void) {
    volatile int bound = 1000;  /* Volatile prevents constant propagation */
    volatile int sink = 0;
    int sum = 0;
    int i = bound;
    
    /* Loop with volatile-derived counter */
    while (i--) {
        sink += i;
        sum += i % 100;
        dummy_side_effect(i);
    }
    
    return sum;
}

/* Main driver that executes all tests and computes checksum */
int main(void) {
    int checksum = 0;
    
    printf("Running loop-doloop pattern tests...\n");
    
    /* Test A: Basic for loop */
    checksum += test_basic_for_loop(1000);
    printf("Test A completed\n");
    
    /* Test B: While loop */
    checksum += test_while_loop(500);
    printf("Test B completed\n");
    
    /* Test C: Nested loops */
    checksum += test_nested_loops(10, 100);
    printf("Test C completed\n");
    
    /* Test D: Parameter counter */
    checksum += test_param_counter(300);
    printf("Test D completed\n");
    
    /* Test E: Mixed counters */
    checksum += test_mixed_counters(200, 100);
    printf("Test E completed\n");
    
    /* Test F: Volatile bound */
    checksum += test_volatile_bound();
    printf("Test F completed\n");
    
    /* Add global volatile to checksum to ensure all side effects are counted */
    checksum += global_volatile;
    
    printf("Final checksum: %d\n", checksum);
    printf("If checksum is non-zero, loops executed successfully.\n");
    
    return 0;
}
