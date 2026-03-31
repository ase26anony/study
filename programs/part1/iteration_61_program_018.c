#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int total) {
    static int storage = 0;
    storage += total;
    return storage;
}

/* Test function with various do-while patterns */
void test_doloop_patterns(void) {
    int total = 0;
    
    /* PATTERN 1: Basic signed int decrement */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        do {
            total += 3;
            bar();
        } while (--s_counter > 0);
    }
    
    /* PATTERN 4: Char type with explicit decrement */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while ((c_counter -= 1) != 0);
    }
    
    /* PATTERN 5: Counter as function parameter */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* PATTERN 6: Loop inside conditional */
    {
        int flag = 1;
        if (flag) {
            int counter = 8;
            do {
                total += 6;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* PATTERN 7: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        do {
            total += 7;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 8: Loop with pointer in body */
    {
        int counter = 7;
        int *ptr = &total;
        do {
            *ptr += 8;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 9: Mixed operations in loop */
    {
        int counter = 6;
        int temp = 0;
        do {
            total += 9;
            temp++;
            bar();
        } while (--counter > 0);
    }
    
    /* NEGATIVE TEST: Should NOT match pattern (post-increment) */
    {
        int counter = 3;
        do {
            total += 10;
            bar();
        } while (counter++ < 2);
    }
    
    /* NEGATIVE TEST: Should NOT match pattern (compare against non-zero) */
    {
        int counter = 4;
        do {
            total += 11;
            bar();
        } while (--counter > 2);
    }
    
    /* PATTERN 10: Volatile counter (may inhibit optimization) */
    {
        volatile int v_counter = 3;
        do {
            total += 12;
            bar();
        } while (--v_counter > 0);
    }
    
    /* PATTERN 11: Nested decrement pattern */
    {
        int outer = 2;
        do {
            int inner = 3;
            do {
                total += 13;
                bar();
            } while (--inner > 0);
        } while (--outer > 0);
    }
    
    /* Store final result */
    accumulate_result(total);
}

/* Main function with multiple test calls */
int main(void) {
    int final_total = 0;
    
    /* Call test multiple times to ensure loops execute */
    for (int i = 0; i < 3; i++) {
        test_doloop_patterns();
    }
    
    /* Create a predictable result */
    final_total = accumulate_result(0);
    printf("Result: %d\n", final_total);
    
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}
