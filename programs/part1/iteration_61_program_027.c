#include <stdio.h>

/* External function to prevent loop removal */
extern void bar(void);

/* Function to accumulate results */
int accumulate(int value) {
    static int total = 0;
    total += value;
    return total;
}

/* Test function with various do-while patterns */
void test_loops(int param_counter) {
    int result = 0;
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        int sum = 0;
        do {
            sum += counter;
            bar();  /* External call prevents optimization */
        } while (--counter > 0);
        result += sum;
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        int prod = 1;
        do {
            prod *= 2;
            bar();
        } while (--u_counter != 0);
        result += prod;
    }
    
    /* Pattern 3: register-qualified short counter */
    {
        register short reg_counter = 25;
        int temp = 0;
        do {
            temp += reg_counter;
            bar();
        } while (--reg_counter > 0);
        result += temp;
    }
    
    /* Pattern 4: char counter in if statement context */
    {
        char char_counter = 10;
        if (result > 0) {
            int local_sum = 0;
            do {
                local_sum += char_counter;
                bar();
            } while (--char_counter > 0);
            result += local_sum;
        }
    }
    
    /* Pattern 5: Counter as function parameter */
    {
        int local_var = 0;
        int param_copy = param_counter;
        if (param_copy > 0) {
            do {
                local_var += param_copy;
                bar();
            } while (--param_copy > 0);
        }
        result += local_var;
    }
    
    /* Pattern 6: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        int once = 0;
        do {
            once = 1;
            bar();
        } while (--single_counter > 0);
        result += once;
    }
    
    /* Pattern 7: Using pointer in loop body */
    {
        int ptr_counter = 15;
        int data[15] = {0};
        int *ptr = data;
        do {
            *ptr = ptr_counter;
            ptr++;
            bar();
        } while (--ptr_counter > 0);
        result += data[0];
    }
    
    /* Pattern 8: Different storage - volatile (should NOT match pattern) */
    {
        volatile int vol_counter = 20;
        int vol_sum = 0;
        do {
            vol_sum += 1;
            bar();
        } while (--vol_counter > 0);
        result += vol_sum;
    }
    
    /* Pattern 9: Counter with subtraction instead of prefix decrement */
    {
        int sub_counter = 30;
        int sub_result = 0;
        do {
            sub_result += sub_counter;
            bar();
            sub_counter -= 1;  /* Different pattern - may not match */
        } while (sub_counter > 0);
        result += sub_result;
    }
    
    /* Pattern 10: Post-increment (should NOT match the -1 pattern) */
    {
        int post_counter = 5;
        int post_sum = 0;
        do {
            post_sum += post_counter;
            bar();
        } while (post_counter-- > 1);  /* Different pattern */
        result += post_sum;
    }
    
    /* Pattern 11: Compare against non-zero (should fail const0_rtx check) */
    {
        int nonzero_counter = 40;
        int nonzero_sum = 0;
        do {
            nonzero_sum += nonzero_counter;
            bar();
        } while (--nonzero_counter > 10);  /* Compare against 10, not 0 */
        result += nonzero_sum;
    }
    
    /* Pattern 12: Nested in complex control flow */
    {
        int complex_counter = 12;
        int complex_result = 0;
        for (int i = 0; i < 3; i++) {
            int inner_counter = complex_counter;
            do {
                complex_result += i * inner_counter;
                bar();
            } while (--inner_counter > 0);
        }
        result += complex_result;
    }
    
    accumulate(result);
}

/* Dummy implementation of bar() for linking */
void bar(void) {
    /* Empty but external - prevents optimization */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int total = 0;
    
    /* Run tests with different parameters */
    test_loops(8);
    test_loops(3);
    test_loops(15);
    
    /* Final accumulation */
    total = accumulate(0);  /* Returns current total */
    
    printf("Result: %d\n", total);
    return 0;
}
