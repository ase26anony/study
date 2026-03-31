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
    
    /* PATTERN 1: Basic signed int decrement (should match pattern) */
    {
        int counter = 100;
        do {
            total += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* PATTERN 2: Unsigned int decrement with != 0 (should match) */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            *((volatile int*)&vol_total) = total;  /* Simple side effect */
        } while (--u_counter != 0);
    }
    
    /* PATTERN 3: Short type with register qualifier (should match) */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while ((reg_counter -= 1) != 0);
    }
    
    /* PATTERN 4: Char type in if statement context */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += 4;
                bar();
            } while (--char_counter > 0);
        }
    }
    
    /* PATTERN 5: Function parameter as counter */
    {
        int local_param = param_counter;
        if (local_param > 0) {
            do {
                total += 5;
                *((volatile int*)&vol_total) = total;
            } while (--local_param > 0);
        }
    }
    
    /* PATTERN 6: Counter starting at 1 (edge case) */
    {
        int edge_counter = 1;
        do {
            total += 6;
            bar();
        } while (--edge_counter > 0);
    }
    
    /* PATTERN 7: Different storage - automatic with pointer */
    {
        int counter = 15;
        int *ptr = &counter;
        do {
            total += 7;
            bar();
        } while ((*ptr -= 1) > 0);
    }
    
    /* NEGATIVE TEST 1: Post-increment (should NOT match pattern) */
    {
        int counter = 5;
        do {
            total += 8;
        } while (counter++ < 10);
    }
    
    /* NEGATIVE TEST 2: Compare against non-zero (should NOT match) */
    {
        int counter = 20;
        do {
            total += 9;
        } while (--counter > 5);
    }
    
    /* PATTERN 8: Mixed operations in loop body */
    {
        unsigned char byte_counter = 30;
        int temp = 0;
        do {
            total += 10;
            temp = total * 2;  /* Additional computation */
            bar();
        } while (--byte_counter != 0);
    }
    
    /* PATTERN 9: Nested context with external call */
    {
        int outer_var = 40;
        if (outer_var > 10) {
            int inner_counter = 12;
            do {
                total += 11;
                bar();
                if (total % 2 == 0) {
                    vol_total = total;  /* Conditional side effect */
                }
            } while (--inner_counter > 0);
        }
    }
    
    /* Final accumulation to ensure loops aren't eliminated */
    accumulate_result(total + vol_total);
}

/* Main function with predictable output */
int main() {
    int i;
    
    /* Call test multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        test_loops(i * 10 + 5);
    }
    
    /* Create one more direct test in main */
    {
        int main_counter = 8;
        int main_total = 0;
        
        do {
            main_total += main_counter;
            bar();
        } while (--main_counter > 0);
        
        printf("Result: %d\n", main_total);
    }
    
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int dummy = 0;
    dummy++;
}
