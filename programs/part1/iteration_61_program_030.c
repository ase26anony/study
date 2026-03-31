#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Global pointer for memory side effects */
int *global_ptr;

/* Function with various do-while loops targeting the RTL pattern */
void test_doloop_patterns(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent some optimizations */
    
    /* Pattern 1: Basic signed int decrement - should match */
    {
        int counter = 100;
        do {
            total += 1;
            bar();  /* External call prevents dead code elimination */
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int decrement with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            total += 2;
            /* Simple memory side effect */
            if (global_ptr) *global_ptr = total;
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: Char type, embedded in if statement */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += 4;
                /* Use volatile to ensure side effect */
                vol_total = total;
            } while (--char_counter != 0);
        }
    }
    
    /* Pattern 5: Counter starts at 1 (boundary case) */
    {
        int single_counter = 1;
        do {
            total += 5;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 6: Function parameter as counter */
    {
        int param = param_counter;
        if (param > 0) {
            do {
                total += 6;
                if (global_ptr) *global_ptr = total;
            } while (--param > 0);
        }
    }
    
    /* Pattern 7: Loop with post-increment (SHOULD NOT MATCH pattern) */
    {
        int post_counter = 5;
        do {
            total += 7;
            bar();
        } while (post_counter++ < 10);  /* Different pattern - plus 1, not minus 1 */
    }
    
    /* Pattern 8: Loop comparing against non-zero (SHOULD NOT MATCH) */
    {
        int non_zero_counter = 20;
        do {
            total += 8;
            bar();
        } while (--non_zero_counter > 5);  /* Compare against 5, not 0 */
    }
    
    /* Pattern 9: Complex expression in condition (might not match) */
    {
        int complex_counter = 15;
        do {
            total += 9;
            vol_total = total;
        } while ((complex_counter -= 1) != 0);  /* Alternative decrement syntax */
    }
    
    /* Pattern 10: Nested context with pointer arithmetic */
    {
        int ptr_counter = 8;
        int array[10];
        int *ptr = array;
        do {
            *ptr++ = total;
            total += 10;
            bar();
        } while (--ptr_counter > 0);
    }
    
    printf("Accumulated total: %d\n", total);
    printf("Volatile total: %d\n", vol_total);
}

/* Dummy implementation of bar() for linking */
void bar(void) {
    /* Empty but external linkage prevents optimization */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int dummy_array[5];
    global_ptr = &dummy_array[0];
    
    /* Test with different parameter values */
    test_doloop_patterns(7);
    test_doloop_patterns(0);
    test_doloop_patterns(3);
    
    return 0;
}
