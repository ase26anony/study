#include <stdio.h>

/* External function to prevent optimization */
extern void bar(void);

/* Function using various do-while patterns */
void test_loops(int param_counter) {
    int total = 0;
    volatile int vol_total = 0;  /* Prevent optimization */
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
            *ptr = *ptr + 1;
        } while (--u_counter != 0);
    }
    
    /* Pattern 3: Short type with register qualifier */
    {
        register short s_counter = 25;
        do {
            vol_total = vol_total + 1;
            bar();
        } while (--s_counter > 0);
    }
    
    /* Pattern 4: Char type in if statement context */
    {
        char c_counter = 10;
        if (total > 0) {
            do {
                total += c_counter;
            } while (--c_counter > 0);
        }
    }
    
    /* Pattern 5: Function parameter as counter */
    {
        int local_copy = param_counter;
        if (local_copy > 0) {
            do {
                bar();
                total += 2;
            } while (--local_copy > 0);
        }
    }
    
    /* Pattern 6: Counter starting at 1 (edge case) */
    {
        int counter = 1;
        do {
            total += 3;
        } while (--counter > 0);
    }
    
    /* Pattern 7: Different storage - automatic with complex expression */
    {
        int counter = 30;
        int temp = 0;
        do {
            temp = counter;
            total += temp;
            bar();
        } while (--counter > 0);
    }
    
    /* Pattern 8: Should NOT match - post-increment instead of decrement */
    {
        int counter = 5;
        do {
            total += 4;
        } while (counter++ < 10);  /* This should fail cmp_arg2 != const0_rtx */
    }
    
    /* Pattern 9: Should NOT match - compare against non-zero value */
    {
        int counter = 15;
        do {
            total += 5;
        } while (--counter > 5);  /* This should fail cmp_arg2 != const0_rtx */
    }
    
    /* Pattern 10: Compound assignment decrement */
    {
        int counter = 20;
        do {
            bar();
            total += 6;
        } while ((counter -= 1) != 0);
    }
    
    /* Pattern 11: Nested context with multiple statements after */
    {
        int counter = 12;
        do {
            total += 7;
            bar();
        } while (--counter > 0);
        
        /* Additional statements affecting register allocation */
        int extra = total * 2;
        vol_total = extra;
    }
    
    /* Pattern 12: Mixed types in comparison */
    {
        unsigned char uc_counter = 8;
        do {
            total += uc_counter;
            bar();
        } while (--uc_counter != 0);
    }
    
    printf("Result: %d (volatile: %d)\n", total, vol_total);
}

/* Dummy implementation of bar() if not linked externally */
void bar(void) {
    /* Empty but non-const/non-pure */
    static int call_count = 0;
    call_count++;
}

int main(void) {
    /* Test with different parameter values */
    test_loops(7);
    test_loops(0);
    test_loops(3);
    
    return 0;
}
