#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Function to accumulate results */
int accumulate_result(int total) {
    static int storage = 0;
    storage += total;
    return storage;
}

/* Test function with various do-while patterns */
void test_loops(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    
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
            *(&vol_total) = 1;  /* Simple side effect */
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: register qualified variable */
    {
        register int reg_counter = 25;
        do {
            total += 3;
            bar();
        } while ((reg_counter -= 1) != 0);
    }
    
    /* PATTERN 4: short type counter */
    {
        short s_counter = 10;
        do {
            total += 4;
            bar();
        } while (--s_counter > 0);
    }
    
    /* PATTERN 5: char type counter */
    {
        char c_counter = 5;
        do {
            total += 5;
            bar();
        } while (--c_counter != 0);
    }
    
    /* PATTERN 6: Counter as function parameter */
    {
        int counter = param_counter > 0 ? param_counter : 10;
        do {
            total += 6;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 7: Loop inside conditional */
    {
        int flag = 1;
        if (flag) {
            int counter = 8;
            do {
                total += 7;
                bar();
            } while (--counter > 0);
        }
    }
    
    /* PATTERN 8: Counter starting at 1 (executes once) */
    {
        int counter = 1;
        do {
            total += 8;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 9: Followed by other statements */
    {
        int counter = 7;
        int local_sum = 0;
        do {
            local_sum += counter;
            bar();
        } while (--counter > 0);
        total += local_sum;
    }
    
    /* NEGATIVE TEST: Should NOT match pattern (post-increment) */
    {
        int counter = 5;
        do {
            total += 9;
            bar();
        } while (counter++ < 10);  /* Post-increment, not decrement */
    }
    
    /* NEGATIVE TEST: Should NOT match pattern (compare against non-zero) */
    {
        int counter = 20;
        do {
            total += 10;
            bar();
        } while (--counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* PATTERN 10: Mixed with other control flow */
    {
        int counter = 6;
        int *ptr = &total;
        do {
            if (counter % 2 == 0) {
                *ptr += 11;
            }
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 11: volatile counter (likely won't match but tests boundary) */
    {
        volatile int vol_counter = 3;
        do {
            total += 12;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* PATTERN 12: unsigned char with != 0 */
    {
        unsigned char uc_counter = 4;
        do {
            total += 13;
            bar();
        } while (--uc_counter != 0);
    }
    
    accumulate_result(total);
}

/* Main function with multiple test calls */
int main() {
    int i;
    
    /* Call test_loops multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        test_loops(i * 5 + 3);
    }
    
    /* Final computation and output */
    printf("Test completed (compile with -O2 -fdump-rtl-doloop)\n");
    printf("Check RTL dumps for doloop pattern matching\n");
    
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}
