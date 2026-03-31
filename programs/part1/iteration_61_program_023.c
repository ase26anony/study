#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
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
    volatile int vol_result = 0;  /* May inhibit pattern */
    
    /* Pattern 1: Basic signed int decrement */
    {
        int counter = 100;
        do {
            result += 1;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int u_counter = 50;
        do {
            result += 2;
            *((volatile int*)&vol_result) = result;
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register qualified variable */
    {
        register int reg_counter = 25;
        do {
            result += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short s_counter = 10;
        do {
            result += 4;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 5: char type counter */
    {
        char c_counter = 5;
        do {
            result += 5;
            *((volatile int*)&vol_result) = result;
        } while (--c_counter > 0);
    }
    
    /* Pattern 6: Counter as function parameter */
    {
        int local_param = param_counter;
        if (local_param > 0) {
            do {
                result += 6;
                bar();
            } while (--local_param > 0);
        }
    }
    
    /* Pattern 7: Counter starting at 1 (edge case) */
    {
        int single_counter = 1;
        do {
            result += 7;
            bar();
        } while (--single_counter > 0);
    }
    
    /* Pattern 8: Inside if statement */
    {
        int if_counter = 8;
        if (result > 0) {
            do {
                result += 8;
                *((volatile int*)&vol_result) = result;
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 9: With explicit subtraction */
    {
        int sub_counter = 12;
        do {
            result += 9;
            bar();
        } while ((sub_counter -= 1) != 0);
    }
    
    /* Pattern 10: Followed by other statements */
    {
        int follow_counter = 6;
        do {
            result += 10;
            bar();
        } while (--follow_counter > 0);
        /* Additional statements affecting register allocation */
        int temp = result * 2;
        *((volatile int*)&vol_result) = temp;
    }
    
    /* NON-MATCHING PATTERNS (should fail the checks) */
    
    /* Pattern A: Post-increment (should not match GEN_INT(-1)) */
    {
        int post_counter = 5;
        do {
            result += 100;
            bar();
        } while (post_counter++ < 10);
    }
    
    /* Pattern B: Compare against non-zero (should fail const0_rtx check) */
    {
        int non_zero_counter = 10;
        do {
            result += 200;
            *((volatile int*)&vol_result) = result;
        } while (--non_zero_counter > 5);
    }
    
    /* Pattern C: Different decrement amount */
    {
        int dec2_counter = 20;
        do {
            result += 300;
            bar();
        } while ((dec2_counter -= 2) > 0);
    }
    
    accumulate(result);
}

/* Main function with multiple test calls */
int main() {
    int i;
    
    /* Call test_loops multiple times with different parameters */
    for (i = 0; i < 3; i++) {
        test_loops(15 + i * 5);
    }
    
    /* Final verification output */
    printf("Test completed. Check RTL dumps for pattern matching.\n");
    printf("Compile with: gcc -O2 -fdump-rtl-doloop -c this_file.c\n");
    
    return 0;
}

/* Dummy implementation of bar() to satisfy linker */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}
