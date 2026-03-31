/* test-doloop.c - Test for doloop optimization pattern matching */
#include <stdio.h>
#include <stdlib.h>

volatile int global_sum = 0;  /* volatile to prevent dead code elimination */

/* Test 1: do-while with pre-decrement */
int test1(int n) {
    int sum = 0;
    int counter = n;
    
    do {
        sum += counter * 2;
        global_sum += 1;
    } while (--counter != 0);
    
    return sum;
}

/* Test 2: while loop with post-decrement */
int test2(int n) {
    int sum = 0;
    int counter = n;
    
    while (counter-- != 0) {
        sum += counter * 3;
        global_sum += 2;
    }
    
    return sum;
}

/* Test 3: unsigned counter */
unsigned int test3(unsigned int n) {
    unsigned int sum = 0;
    unsigned int counter = n;
    
    do {
        sum += counter;
        global_sum += 3;
    } while (--counter != 0);
    
    return sum;
}

/* Test 4: Nested loops - inner loop should generate the pattern */
int test4(int outer, int inner) {
    int sum = 0;
    
    for (int i = 0; i < outer; i++) {
        int counter = inner;
        
        do {
            sum += (i * counter);
            global_sum += 4;
        } while (--counter != 0);
    }
    
    return sum;
}

/* Test 5: Simple decrementing loop with array access */
int test5(int n) {
    int sum = 0;
    int arr[100];
    
    /* Initialize array */
    for (int i = 0; i < 100; i++) {
        arr[i] = i;
    }
    
    int counter = n;
    while (counter != 0) {
        sum += arr[counter % 100];
        counter--;  /* Decrement in loop body, check in condition */
    }
    
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <loop_count>\n", argv[0]);
        return 1;
    }
    
    int base_count = atoi(argv[1]);
    if (base_count <= 0) {
        base_count = 10;  /* Default if invalid */
    }
    
    int total = 0;
    
    /* Run all tests with different loop counts to ensure execution */
    total += test1(base_count);
    total += test2(base_count);
    total += test3((unsigned int)base_count);
    total += test4(3, base_count);
    total += test5(base_count);
    
    printf("Total: %d, Global sum: %d\n", total, global_sum);
    
    /* Return predictable value for verification */
    return (total > 0) ? 0 : 1;
}
