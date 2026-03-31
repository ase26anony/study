#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int total) {
    static int storage = 0;
    storage += total;
    return storage;
}

/* Test various do-while patterns */
void test_doloop_patterns(void) {
    int total = 0;
    
    /* Pattern 1: Basic signed int decrement */
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
            total += 2;
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
    
    /* Pattern 4: Char type with explicit subtraction */
    {
        char c_counter = 10;
        do {
            total += 4;
            bar();
        } while ((c_counter -= 1) != 0);
    }
    
    /* Pattern 5: Counter as function parameter */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* Pattern 6: Loop in if statement context */
    {
        int flag = 1;
        if (flag) {
            int if_counter = 8;
            do {
                total += 6;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int edge_counter = 1;
        do {
            total += 7;
            bar();
        } while (--edge_counter > 0);
    }
    
    /* Pattern 8: Followed by other statements */
    {
        int follow_counter = 3;
        do {
            total += 8;
            bar();
        } while (--follow_counter > 0);
        /* Additional statement affecting register allocation */
        int temp = total * 2;
        (void)temp;
    }
    
    /* NON-MATCHING PATTERNS (should not trigger the uncovered code) */
    
    /* Pattern 9: Post-increment (should NOT match) */
    {
        int post_counter = 5;
        do {
            total += 9;
            bar();
        } while (post_counter++ < 10);
    }
    
    /* Pattern 10: Compare against non-zero (should NOT match) */
    {
        int non_zero_counter = 10;
        do {
            total += 10;
            bar();
        } while (--non_zero_counter > 5);
    }
    
    /* Pattern 11: Volatile counter (likely won't match) */
    {
        volatile int vol_counter = 4;
        do {
            total += 11;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern 12: Different operation in condition */
    {
        int diff_counter = 6;
        do {
            total += 12;
            bar();
        } while ((diff_counter = diff_counter - 2) > 0);
    }
    
    accumulate_result(total);
}

/* Additional test with pointer in loop body */
void test_with_pointers(void) {
    int data[10] = {0};
    int *ptr = data;
    int p_counter = 10;
    
    do {
        *ptr = 1;
        ptr++;
        bar();
    } while (--p_counter > 0);
    
    /* Verify the pattern with different types */
    unsigned short us_counter = 20;
    do {
        data[0] += us_counter;
        bar();
    } while (--us_counter != 0);
}

/* Main function with multiple test cases */
int main(void) {
    int final_total = 0;
    
    /* Run pattern tests multiple times */
    for (int i = 0; i < 3; i++) {
        test_doloop_patterns();
        test_with_pointers();
        final_total += i * 100;
    }
    
    /* Print predictable result */
    printf("Result: %d\n", final_total);
    
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Minimal side effect */
    static int call_count = 0;
    call_count++;
}
