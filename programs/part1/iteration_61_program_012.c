#include <stdio.h>

/* External function to prevent loop elimination */
extern void bar(void);

/* Global pointer for side effects */
volatile int *global_ptr;

/* Function to test loops with parameter counter */
int test_param_counter(int param_counter) {
    int sum = 0;
    do {
        sum += param_counter;
        bar();
    } while (--param_counter > 0);
    return sum;
}

int main() {
    int total = 0;
    volatile int *ptr = &total;
    
    /* Pattern 1: Basic signed int counter - should match */
    {
        int counter = 100;
        do {
            total += counter;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int counter with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            *ptr = 0;  /* Simple side effect */
            bar();
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register-qualified short counter */
    {
        register short reg_counter = 25;
        do {
            total += reg_counter;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: char counter in if statement context */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += char_counter;
                bar();
            } while (--char_counter > 0);
        }
    }
    
    /* Pattern 5: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            total += 100;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 6: Function parameter counter */
    total += test_param_counter(5);
    
    /* Pattern 7: Counter with post-increment (SHOULD NOT MATCH) */
    {
        int post_counter = 10;
        int temp = 0;
        do {
            temp += post_counter;
            bar();
        } while (post_counter-- > 0);
        total += temp;
    }
    
    /* Pattern 8: Counter comparing against non-zero (SHOULD NOT MATCH) */
    {
        int non_zero_counter = 20;
        do {
            total += 2;
            bar();
        } while (--non_zero_counter > 5);
    }
    
    /* Pattern 9: volatile counter (likely won't match but tests edge) */
    {
        volatile int vol_counter = 15;
        do {
            total += 3;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern 10: Complex context with multiple statements */
    {
        int context_counter = 30;
        int local_sum = 0;
        do {
            local_sum += context_counter;
            bar();
        } while (--context_counter > 0);
        total += local_sum;
        
        /* Additional statements affecting register allocation */
        int extra = local_sum * 2;
        total += extra;
    }
    
    /* Pattern 11: Different decrement syntax */
    {
        int alt_counter = 40;
        do {
            total += alt_counter;
            bar();
        } while ((alt_counter -= 1) != 0);
    }
    
    /* Pattern 12: unsigned short with post-decrement in condition */
    {
        unsigned short us_counter = 60;
        do {
            total += us_counter;
            bar();
        } while (us_counter-- != 0);
        total -= 60; /* Adjust for extra iteration */
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    static int dummy = 0;
    dummy++;
}
