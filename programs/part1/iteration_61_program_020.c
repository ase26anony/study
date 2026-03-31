#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Global pointer for side effects */
int global_sum = 0;

/* Function with various do-while loops targeting the specific RTL pattern */
void test_doloop_patterns(void) {
    int total = 0;
    volatile int vol_counter; /* May inhibit pattern */
    
    /* PATTERN 1: Basic signed int decrement - should match */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int with != 0 comparison - should match */
    {
        unsigned int u_counter = 50;
        do {
            global_sum += 2;
            bar();
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with register qualifier - should match */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* PATTERN 4: Char type - should match */
    {
        char char_counter = 10;
        do {
            global_sum += 4;
            bar();
        } while (--char_counter != 0);
    }
    
    /* PATTERN 5: Counter as function parameter */
    {
        int param_counter = 5;
        do {
            total += 5;
            bar();
        } while (--param_counter > 0);
    }
    
    /* PATTERN 6: Inside if statement */
    {
        int flag = 1;
        if (flag) {
            int if_counter = 8;
            do {
                global_sum += 6;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* PATTERN 7: Counter starts at 1 (edge case) */
    {
        int edge_counter = 1;
        do {
            total += 7;
            bar();
        } while (--edge_counter > 0);
    }
    
    /* PATTERN 8: With pointer side effect */
    {
        int ptr_counter = 3;
        int *ptr = &total;
        do {
            *ptr += 8;
            bar();
        } while (--ptr_counter > 0);
    }
    
    /* PATTERN 9: Followed by other statements */
    {
        int follow_counter = 4;
        do {
            global_sum += 9;
            bar();
        } while (--follow_counter > 0);
        total += 100; /* Additional statement */
    }
    
    /* PATTERN 10: Nested in another control flow */
    {
        for (int i = 0; i < 2; i++) {
            int nested_counter = 2;
            do {
                total += 10;
                bar();
            } while (--nested_counter > 0);
        }
    }
    
    /* NEGATIVE TEST 1: Post-increment (should NOT match) */
    {
        int post_counter = 5;
        do {
            total += 11;
            bar();
        } while (post_counter++ < 10);
    }
    
    /* NEGATIVE TEST 2: Compare against non-zero (should NOT match) */
    {
        int non_zero_counter = 10;
        do {
            global_sum += 12;
            bar();
        } while (--non_zero_counter > 5);
    }
    
    /* NEGATIVE TEST 3: Volatile counter (likely won't match) */
    {
        vol_counter = 3;
        do {
            total += 13;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* PATTERN 11: Using subtraction instead of prefix decrement */
    {
        int sub_counter = 7;
        do {
            global_sum += 14;
            bar();
        } while ((sub_counter -= 1) != 0);
    }
    
    /* PATTERN 12: Mixed types in comparison */
    {
        short mixed_counter = 6;
        do {
            total += 15;
            bar();
        } while (--mixed_counter != 0);
    }
    
    printf("Total: %d, Global sum: %d\n", total, global_sum);
}

/* Dummy implementation of bar() to avoid linker errors */
void bar(void) {
    /* Empty but non-const/non-pure */
    asm volatile("" ::: "memory");
}

int main(void) {
    test_doloop_patterns();
    return 0;
}
