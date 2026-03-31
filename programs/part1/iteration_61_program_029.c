#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function to accumulate results */
int accumulate(int *total, int value) {
    *total += value;
    return *total;
}

/* Main function containing various do-while patterns */
int main() {
    int total = 0;
    int *ptr = &total;
    
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
            *ptr = *ptr + 2;
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: register qualified variable */
    {
        register int reg_counter = 25;
        do {
            total += 3;
            bar();
        } while ((reg_counter -= 1) != 0);
    }
    
    /* Pattern 4: short type counter */
    {
        short s_counter = 10;
        do {
            accumulate(&total, 4);
        } while (--s_counter > 0);
    }
    
    /* Pattern 5: char type counter */
    {
        char c_counter = 5;
        do {
            total += 5;
            bar();
        } while (--c_counter != 0);
    }
    
    /* Pattern 6: Counter starting at 1 (executes once) */
    {
        int once_counter = 1;
        do {
            total += 10;
        } while (--once_counter > 0);
    }
    
    /* Pattern 7: Inside if statement */
    {
        int if_counter = 8;
        if (total < 1000) {
            do {
                total += 6;
                bar();
            } while (--if_counter > 0);
        }
    }
    
    /* Pattern 8: With volatile (should NOT match pattern) */
    {
        volatile int vol_counter = 3;
        do {
            total += 7;
        } while (--vol_counter > 0);
    }
    
    /* Pattern 9: Function parameter as counter */
    {
        int param_counter = 7;
        void nested_func(int cnt) {
            do {
                total += 8;
                bar();
            } while (--cnt > 0);
        }
        nested_func(param_counter);
    }
    
    /* Pattern 10: Followed by other statements affecting liveness */
    {
        int live_counter = 6;
        int temp = 0;
        do {
            temp += 9;
            bar();
        } while (--live_counter > 0);
        total += temp;
    }
    
    /* NON-MATCHING PATTERNS (for boundary testing) */
    
    /* Pattern A: Post-increment (should NOT match) */
    {
        int post_counter = 4;
        do {
            total += 11;
        } while (post_counter++ < 10);
    }
    
    /* Pattern B: Compare against non-zero (should NOT match) */
    {
        int non_zero_counter = 3;
        do {
            total += 12;
        } while (--non_zero_counter > 2);
    }
    
    /* Pattern C: Complex expression in condition */
    {
        int complex_counter = 5;
        int other = 2;
        do {
            total += 13;
        } while ((complex_counter -= other) > 0);
    }
    
    /* Final result */
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    if (total > 1000) {
        bar();
    }
    
    return 0;
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    static int dummy = 0;
    dummy++;
}
