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
        int counter1 = 100;
        do {
            total += 1;
            bar();
        } while (--counter1 > 0);
    }
    
    /* Pattern 2: Unsigned int with != 0 comparison */
    {
        unsigned int counter2 = 50;
        do {
            *ptr = *ptr + 2;
        } while (--counter2 != 0);
    }
    
    /* Pattern 3: register-qualified char counter */
    {
        register char counter3 = 25;
        do {
            total += 3;
            bar();
        } while (--counter3 > 0);
    }
    
    /* Pattern 4: short counter in if context */
    {
        short counter4 = 10;
        if (total < 1000) {
            do {
                total += 4;
            } while (--counter4 > 0);
        }
    }
    
    /* Pattern 5: Counter starting at 1 (edge case) */
    {
        int counter5 = 1;
        do {
            total += 5;
            bar();
        } while (--counter5 > 0);
    }
    
    /* Pattern 6: Counter as function parameter simulation */
    {
        int process_count(int count) {
            int local_total = 0;
            do {
                local_total += count;
                bar();
            } while (--count > 0);
            return local_total;
        }
        total += process_count(8);
    }
    
    /* Pattern 7: Different storage - automatic with complex expression */
    {
        volatile int counter7 = 15;  /* May not match pattern */
        do {
            total += 7;
        } while ((counter7 -= 1) != 0);
    }
    
    /* Pattern 8: Nested in control flow with post-statements */
    {
        int counter8 = 12;
        do {
            total += 8;
        } while (--counter8 > 0);
        
        /* Additional statements affecting register allocation */
        int temp = total * 2;
        total = temp / 2;
    }
    
    /* Pattern 9: Should NOT match - post-increment */
    {
        int counter9 = 20;
        int acc = 0;
        do {
            acc += 9;
        } while (counter9++ < 19);  /* Wrong pattern for doloop */
        total += acc;
    }
    
    /* Pattern 10: Should NOT match - compare against non-zero */
    {
        int counter10 = 30;
        do {
            total += 10;
        } while (--counter10 > 5);  /* cmp_arg2 != const0_rtx check fails */
    }
    
    /* Pattern 11: unsigned short with simple body */
    {
        unsigned short counter11 = 18;
        do {
            bar();
        } while (--counter11 != 0);
    }
    
    /* Pattern 12: Counter modification in PLUS form */
    {
        int counter12 = 22;
        int decrement_result;
        do {
            total += 12;
            decrement_result = counter12 - 1;
        } while ((counter12 = decrement_result) > 0);
    }
    
    /* Pattern 13: Multiple counters to stress register allocation */
    {
        int counter13a = 7, counter13b = 7;
        do {
            total += 13;
            bar();
        } while (--counter13a > 0);
        
        do {
            total += 14;
        } while (--counter13b > 0);
    }
    
    printf("Result: %d\n", total);
    return 0;
}

/* Dummy implementation of bar() to allow linking */
void bar(void) {
    static int dummy = 0;
    dummy++;
}
