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
    volatile int vol_total = 0;  /* Prevent optimization */
    int *ptr = &total;
    
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
            *ptr = *ptr + 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with register qualifier */
    {
        register short reg_counter = 25;
        do {
            vol_total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* PATTERN 4: Char type in if statement context */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += 4;
                bar();
            } while (--char_counter != 0);
        }
    }
    
    /* PATTERN 5: Counter as function parameter pattern */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while ((param_counter -= 1) != 0);
    }
    
    /* PATTERN 6: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            total += 6;
            bar();
        } while (--single_counter > 0);
    }
    
    /* PATTERN 7: Different storage class combinations */
    {
        auto int auto_counter = 8;
        register int reg_counter2 = 8;
        do {
            total += 7;
            bar();
        } while (--auto_counter > 0);
        
        /* Separate loop with register */
        do {
            vol_total += 8;
            bar();
        } while (--reg_counter2 != 0);
    }
    
    /* PATTERN 8: Loop with post-increment (SHOULD NOT MATCH) */
    {
        int post_counter = 3;
        do {
            total += 9;
            bar();
        } while (post_counter++ < 10);  /* Different pattern */
    }
    
    /* PATTERN 9: Loop comparing against non-zero (SHOULD NOT MATCH) */
    {
        int non_zero_counter = 20;
        do {
            total += 10;
            bar();
        } while (--non_zero_counter > 5);  /* Not comparing against 0 */
    }
    
    /* PATTERN 10: Complex expression in condition */
    {
        int expr_counter = 7;
        int temp = 0;
        do {
            total += 11;
            temp = expr_counter;
            bar();
        } while (--expr_counter != 0);
    }
    
    /* Final accumulation */
    total += accumulate_result(total);
    printf("Result: %d\n", total);
}

/* Main function with additional context */
int main(void) {
    /* Initialize with some computation */
    int init = 0;
    for (int i = 0; i < 10; i++) {
        init += i;
    }
    
    /* Call the test function multiple times */
    test_doloop_patterns();
    
    /* Additional loop in main for context */
    {
        int main_counter = 3;
        int main_total = 0;
        do {
            main_total += main_counter;
            bar();
        } while (--main_counter > 0);
        
        printf("Main total: %d\n", main_total);
    }
    
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}
