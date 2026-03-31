#include <stdio.h>
#include <stdlib.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int accumulate_results(void) {
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
            *((volatile int*)&total) = total; /* Simple side effect */
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register-qualified short counter */
    {
        register short reg_counter = 25;
        do {
            total += 3;
            bar();
        } while (--reg_counter > 0);
    }
    
    /* Pattern 4: char counter in if statement context */
    {
        char char_counter = 10;
        if (total > 0) {
            do {
                total += 4;
                bar();
            } while (--char_counter > 0);
        }
    }
    
    /* Pattern 5: Counter as function parameter (simulated) */
    {
        int param_counter = 5;
        /* Simulate parameter context */
        {
            do {
                total += 5;
                *((volatile int*)&total) = total;
            } while (--param_counter > 0);
        }
    }
    
    /* Pattern 6: Counter starting at 1 (executes once) */
    {
        int once_counter = 1;
        do {
            total += 6;
            bar();
        } while (--once_counter > 0);
    }
    
    /* Pattern 7: Counter with explicit subtraction */
    {
        int sub_counter = 15;
        do {
            total += 7;
            bar();
        } while ((sub_counter -= 1) != 0);
    }
    
    /* Pattern 8: NEGATIVE TEST - post-increment (should NOT match) */
    {
        int post_counter = 8;
        do {
            total += 8;
            bar();
        } while (post_counter++ < 7);
    }
    
    /* Pattern 9: NEGATIVE TEST - compare against non-zero (should NOT match) */
    {
        int non_zero_counter = 12;
        do {
            total += 9;
            bar();
        } while (--non_zero_counter > 5);
    }
    
    /* Pattern 10: volatile counter (likely won't match but tests edge) */
    {
        volatile int vol_counter = 3;
        do {
            total += 10;
            bar();
        } while (--vol_counter > 0);
    }
    
    /* Pattern 11: Mixed context with other statements */
    {
        int mixed_counter = 7;
        int temp = total;
        do {
            temp += 11;
            bar();
        } while (--mixed_counter > 0);
        total += temp;
    }
    
    /* Pattern 12: Long type for different register width */
    {
        long long_counter = 20L;
        do {
            total += 12;
            *((volatile int*)&total) = total;
        } while (--long_counter > 0);
    }
    
    return total;
}

/* Dummy implementation of bar() */
void bar(void) {
    /* Empty but external linkage prevents optimization */
    static int dummy = 0;
    dummy++;
}

int main(void) {
    int result = accumulate_results();
    
    /* Print predictable result */
    printf("Result: %d\n", result);
    
    /* Additional verification */
    if (result > 0) {
        printf("Loops executed successfully\n");
    }
    
    return 0;
}
