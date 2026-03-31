#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void bar(void);

/* Global pointer for memory operations */
volatile int* global_ptr;

/* Function with various do-while loops targeting the RTL pattern */
int test_doloop_patterns(int param_counter) {
    int total = 0;
    volatile int* ptr = &total;
    
    /* Pattern 1: Basic signed int decrement-and-compare */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = *ptr + 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char type, embedded in if statement */
    if (total > 0) {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while (--c_counter != 0);
    }
    
    /* Pattern 5: Counter as function parameter */
    if (param_counter > 0) {
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 6: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            total += 6;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 7: Different storage - automatic with complex expression */
    {
        int counter = 15;
        int temp = 0;
        do {
            temp = counter;
            total += temp;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 8: Nested context with follow-up statements */
    {
        int counter = 20;
        int local_sum = 0;
        do {
            local_sum += counter;
            bar();
        } while (--counter > 0);
        total += local_sum;
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment instead of decrement */
    {
        int counter = 5;
        do {
            total += 7;
            bar();
        } while (counter++ < 10);  /* Should not match: not GEN_INT(-1) */
    }
    
    /* Pattern B: Compare against non-zero value */
    {
        int counter = 10;
        do {
            total += 8;
            bar();
        } while (--counter > 5);  /* Should not match: cmp_arg2 != const0_rtx */
    }
    
    /* Pattern C: Volatile counter (likely inhibits pattern) */
    {
        volatile int v_counter = 5;
        do {
            total += 9;
            bar();
        } while (--v_counter > 0);
    }
    
    /* Pattern D: Different decrement pattern */
    {
        int counter = 10;
        do {
            total += 10;
            bar();
            counter -= 1;
        } while (counter != 0);  /* Might not match the exact PLUS pattern */
    }
    
    return total;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy;
    dummy++;
}

int main(void) {
    int result = test_doloop_patterns(8);
    printf("Result: %d\n", result);
    
    /* Additional test with different contexts */
    {
        int counter = 12;
        int sum = 0;
        do {
            sum += counter;
            bar();
        } while (--counter > 0);
        printf("Additional sum: %d\n", sum);
    }
    
    return 0;
}
