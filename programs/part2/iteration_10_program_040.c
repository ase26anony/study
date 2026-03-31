/* test_loop_doloop.c - Target for GCC loop-doloop.cc coverage */

#include <stdio.h>
#include <stdlib.h>

/* External array to prevent dead code elimination */
extern int extern_array[10000];

/* Dummy function with side effects */
void __attribute__((noinline)) dummy_side_effect(int value) {
    /* Use volatile to ensure the call isn't optimized away */
    volatile static int sink = 0;
    sink += value;
}

/* Function A: Basic for loop with int counter */
int __attribute__((optimize("O2"))) test_for_loop_decrement(int iterations) {
    volatile int checksum = 0;
    
    /* Canonical decrement-and-compare pattern: i-- > 0 */
    for (int i = iterations; i-- > 0;) {
        checksum += i;
        dummy_side_effect(i);
    }
    
    return checksum;
}

/* Function B: While loop with unsigned counter */
unsigned int __attribute__((optimize("O2"))) test_while_loop_decrement(unsigned int n) {
    volatile unsigned int checksum = 0;
    
    /* Another decrement-and-compare pattern: n-- */
    while (n--) {
        checksum += n;
        extern_array[n % 1000] = n;  /* External side effect */
    }
    
    return checksum;
}

/* Function C: Nested loops with inner decrement pattern */
int __attribute__((optimize("O2"))) test_nested_loops(int outer, int inner) {
    volatile int total = 0;
    
    for (int o = 0; o < outer; o++) {
        /* Inner loop uses decrement-and-compare pattern */
        int i = inner;
        while (i-- > 0) {
            total += o * i;
            dummy_side_effect(o + i);
        }
    }
    
    return total;
}

/* Function D: Loop with parameter counter (prevents constant propagation) */
int __attribute__((optimize("O2"))) test_param_counter(int count) {
    volatile int result = 0;
    
    /* Use volatile to hide loop bound from optimizer */
    volatile int bound = count;
    int i = bound;
    
    /* Decrement-and-compare with parameterized bound */
    do {
        result += i;
        extern_array[i % 1000] = result;
    } while (i-- > 0);
    
    return result;
}

/* Function E: Mixed counter types to test different RTL representations */
long __attribute__((optimize("O2"))) test_mixed_counters(short s_count, long l_count) {
    volatile long checksum = 0;
    
    /* First loop with short counter */
    short s = s_count;
    while (s-- > 0) {
        checksum += s;
        dummy_side_effect(s);
    }
    
    /* Second loop with long counter */
    long l = l_count;
    for (; l-- > 0;) {
        checksum += l;
        extern_array[l % 1000] = (int)l;
    }
    
    return checksum;
}

/* Function F: Loop with volatile counter to force memory operations */
int __attribute__((optimize("O1"))) test_volatile_counter(int base) {
    volatile int counter = base;
    volatile int sum = 0;
    
    /* The volatile counter should still produce decrement-and-compare */
    while (counter-- > 0) {
        sum += counter;
        dummy_side_effect(sum);
    }
    
    return sum;
}

/* Main driver that ensures all loops execute */
int main(void) {
    int final_checksum = 0;
    
    printf("Testing loop-doloop pattern matching...\n");
    
    /* Test Function A */
    int result_a = test_for_loop_decrement(1000);
    final_checksum += result_a;
    printf("Test A (for loop): %d\n", result_a);
    
    /* Test Function B */
    unsigned int result_b = test_while_loop_decrement(500);
    final_checksum += (int)result_b;
    printf("Test B (while loop): %u\n", result_b);
    
    /* Test Function C */
    int result_c = test_nested_loops(10, 100);
    final_checksum += result_c;
    printf("Test C (nested loops): %d\n", result_c);
    
    /* Test Function D */
    int result_d = test_param_counter(300);
    final_checksum += result_d;
    printf("Test D (parameter counter): %d\n", result_d);
    
    /* Test Function E */
    long result_e = test_mixed_counters(50, 200);
    final_checksum += (int)result_e;
    printf("Test E (mixed counters): %ld\n", result_e);
    
    /* Test Function F */
    int result_f = test_volatile_counter(150);
    final_checksum += result_f;
    printf("Test F (volatile counter): %d\n", result_f);
    
    printf("Final checksum: %d\n", final_checksum);
    
    /* Verify all loops executed by checking extern_array was touched */
    int verify = 0;
    for (int i = 0; i < 1000; i++) {
        verify += extern_array[i];
    }
    printf("Array verification: %d\n", verify);
    
    return final_checksum != 0 ? 0 : 1;
}

/* Define the external array */
int extern_array[10000] = {0};
